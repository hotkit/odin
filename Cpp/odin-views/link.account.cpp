/**
    Copyright 2018-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <odin/app.hpp>
#include <odin/odin.hpp>
#include <odin/views.hpp>

#include <fost/crypto>
#include <fost/insert>
#include <fost/json>
#include <fost/log>
#include <fostgres/sql.hpp>


namespace {

    inline std::pair<boost::shared_ptr<fostlib::mime>, int>
            respond(fostlib::string message, int code = 403) {
        fostlib::json ret;
        if (not message.empty())
            fostlib::insert(ret, "message", std::move(message));
        fostlib::mime::mime_headers headers;
        boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                fostlib::json::unparse(ret, true), headers, "text/plain"));
        return std::make_pair(response, code);
    }

    bool has_account_registered(
            fostlib::pg::connection &cnx, fostlib::string const identity_id) {
        auto const credentials =
                cnx.procedure(
                           "SELECT 1 FROM "
                           "odin.identity id "
                           "LEFT JOIN odin.credentials cred "
                           "ON cred.identity_id = id.id "
                           "LEFT JOIN odin.facebook_credentials fb "
                           "ON fb.identity_id = id.id "
                           "LEFT JOIN odin.google_credentials gg "
                           "ON gg.identity_id = id.id "
                           "WHERE id.id=$1 "
                           "AND (cred.identity_id IS NOT NULL "
                           "OR fb.identity_id IS NOT NULL "
                           "OR gg.identity_id IS NOT NULL);")
                        .exec(std::vector<fostlib::string>{identity_id});
        return credentials.begin() != credentials.end();
    }

    std::vector<f5::byte> load_key(
            fostlib::pg::connection &cnx,
            fostlib::json jwt_header,
            fostlib::json jwt_body) {
        if (not jwt_body.has_key("sub")) {
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "No subject in the JWT");
        }
        auto const app_token = odin::c_jwt_secret.value();
        return std::vector<f5::byte>(
                app_token.data().begin(), app_token.data().end());
    }

    std::vector<f5::byte> load_app_key(
            fostlib::pg::connection &cnx,
            fostlib::json jwt_header,
            fostlib::json jwt_body) {
        if (not jwt_body.has_key("sub")) {
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "No subject in the JWT");
        }
        if (not jwt_body.has_key("iss")) {
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "No issuer in the JWT");
        }
        auto const jwt_iss = fostlib::coerce<fostlib::string>(jwt_body["iss"]);
        if (not jwt_iss.startswith(odin::c_app_namespace.value())) {
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "App namespace prefix does not match");
        }
        auto const app_id =
                jwt_iss.substr(odin::c_app_namespace.value().code_points());
        fostlib::json app = odin::app::get_detail(cnx, app_id);
        if (app.isnull()) {
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "App not found");
        }
        auto const app_token = odin::c_jwt_secret.value() + app_id;
        return std::vector<f5::byte>(
                app_token.data().begin(), app_token.data().end());
    }

    fostlib::nullable<fostlib::jwt::token>
            load_jwt(fostlib::pg::connection &cnx, fostlib::string jwt_string) {
        auto const jwt = fostlib::jwt::token::load(
                jwt_string, [&cnx](fostlib::json h, fostlib::json b) {
                    return load_key(cnx, h, b);
                });
        return jwt;
    }

    fostlib::nullable<fostlib::jwt::token> load_app_jwt(
            fostlib::pg::connection &cnx, fostlib::string jwt_string) {
        auto const jwt = fostlib::jwt::token::load(
                jwt_string, [&cnx](fostlib::json h, fostlib::json b) {
                    return load_app_key(cnx, h, b);
                });
        if (not jwt) {
            fostlib::log::debug(odin::c_odin)("detail", "JWT not valid")(
                    "jwt", jwt_string);
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "JWT not valid");
        }
        return jwt;
    }

    fostlib::json app_user_detail_from_jwt(
            fostlib::pg::connection &cnx,
            fostlib::nullable<fostlib::jwt::token> jwt) {
        auto const app_id =
                fostlib::coerce<fostlib::string>(jwt.value().payload["iss"])
                        .substr(odin::c_app_namespace.value().code_points());
        auto const app_user_id =
                fostlib::coerce<fostlib::string>(jwt.value().payload["sub"]);
        auto const identity_id =
                odin::app::get_app_user_identity_id(cnx, app_id, app_user_id);

        if (not identity_id) {
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "App user not found");
        }
        fostlib::json app_user_detail;
        fostlib::insert(app_user_detail, "identity_id", identity_id);
        fostlib::insert(app_user_detail, "app_id", app_id);
        fostlib::insert(app_user_detail, "app_user_id", app_user_id);
        return app_user_detail;
    }

    const class link_account : public fostlib::urlhandler::view {
      public:
        link_account() : view("odin.link.account") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                fostlib::json const &config,
                fostlib::string const &path,
                fostlib::http::server::request &req,
                fostlib::host const &host) const {
            if (req.method() != "POST") {
                return respond("Link Account require POST.", 405);
            }

            auto const body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            auto const body = fostlib::json::parse(body_str);
            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            fostlib::string from_identity_id;
            fostlib::string to_identity_id;

            try {
                if (not body.has_key("from_account")
                    || not body.has_key("to_account")) {
                    throw fostlib::exceptions::not_implemented(
                            __PRETTY_FUNCTION__,
                            "Must pass from_account and to_account field");
                }
                // from_account jwt must be app_token
                auto const from_account_jwt = load_app_jwt(
                        cnx,
                        fostlib::coerce<fostlib::string>(body["from_account"]));
                fostlib::json from_account_detail =
                        app_user_detail_from_jwt(cnx, from_account_jwt);
                from_identity_id = fostlib::coerce<fostlib::string>(
                        from_account_detail["identity_id"]);
                // to_account jwt could be user_token or user_token
                auto const to_account_jwt = load_jwt(
                        cnx,
                        fostlib::coerce<fostlib::string>(body["to_account"]));
                if (to_account_jwt) {
                    to_identity_id = fostlib::coerce<fostlib::string>(
                            to_account_jwt.value().payload["sub"]);
                } else {
                    auto const to_account_app_jwt = load_app_jwt(
                            cnx,
                            fostlib::coerce<fostlib::string>(
                                    body["to_account"]));
                    fostlib::json to_account_detail =
                            app_user_detail_from_jwt(cnx, to_account_app_jwt);
                    to_identity_id = fostlib::coerce<fostlib::string>(
                            to_account_detail["identity_id"]);
                }
                // Two user already merged
                if (from_identity_id == to_identity_id) {
                    fostlib::mime::mime_headers headers;
                    boost::shared_ptr<fostlib::mime> response(
                            new fostlib::text_body(
                                    fostlib::coerce<fostlib::string>(
                                            body["to_account"]),
                                    headers, "application/jwt"));
                    return std::make_pair(response, 200);
                }

                auto const app_user = odin::app::get_app_user(
                        cnx,
                        fostlib::coerce<fostlib::string>(
                                from_account_detail["app_id"]),
                        to_identity_id);
                if (not app_user.isnull()) {
                    throw fostlib::exceptions::not_implemented(
                            __PRETTY_FUNCTION__,
                            "The to_account has been linked with app.");
                }

                if (has_account_registered(cnx, from_identity_id)) {
                    throw fostlib::exceptions::not_implemented(
                            __PRETTY_FUNCTION__,
                            "The from_account has been registered.");
                }

                if (not has_account_registered(cnx, to_identity_id)) {
                    throw fostlib::exceptions::not_implemented(
                            __PRETTY_FUNCTION__,
                            "The to_account has not been registered.");
                }
            } catch (fostlib::exceptions::not_implemented &e) {
                return respond(
                        fostlib::coerce<fostlib::string>(e.data()["message"]),
                        422);
            }

            fostlib::json merge;
            fostlib::insert(merge, "from_identity_id", from_identity_id);
            fostlib::insert(merge, "to_identity_id", to_identity_id);
            cnx.insert("odin.merge_ledger", merge);
            cnx.commit();


            fostlib::mime::mime_headers headers;
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    fostlib::coerce<fostlib::string>(body["to_account"]),
                    headers, "application/jwt"));
            return std::make_pair(response, 200);
        }

    } c_link_account;
}

const fostlib::urlhandler::view &odin::view::link_account = c_link_account;
