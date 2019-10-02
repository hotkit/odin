/**
    Copyright 2018-2019 Red Anchor Trading Co. Ltd.

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


namespace {


    bool has_account_merged_to_another_account(
        fostlib::pg::connection &cnx,
        fostlib::string const identity_id) {
        auto const merge_ledger = cnx.exec(
            "SELECT 1 FROM "
            "odin.merge_ledger "
            "WHERE from_identity_id='" + identity_id + "';"
        );
        return merge_ledger.begin() != merge_ledger.end();
    }

    bool has_account_merged_from_another_account(
        fostlib::pg::connection &cnx,
        fostlib::string const identity_id) {
        auto const merge_ledger = cnx.exec(
            "SELECT 1 FROM "
            "odin.merge_ledger "
            "WHERE to_identity_id='" + identity_id + "';"
        );
        return merge_ledger.begin() != merge_ledger.end();
    }

    bool has_account_merged(
        fostlib::pg::connection &cnx,
        fostlib::string const identity_id) {
        auto const merge_ledger = cnx.exec(
            "SELECT 1 FROM "
            "odin.merge_ledger "
            "WHERE from_identity_id='" + identity_id
            + "' OR to_identity_id='" + identity_id + "';"
        );
        return merge_ledger.begin() != merge_ledger.end();
    }

    bool has_account_registerd(
        fostlib::pg::connection &cnx,
        fostlib::string const identity_id) {
        auto const credentials = cnx.exec(
            "SELECT 1 FROM "
            "odin.identity id "
            "LEFT JOIN odin.credentials cred "
            "ON cred.identity_id = id.id "
            "LEFT JOIN odin.facebook_credentials fb "
            "ON fb.identity_id = id.id "
            "LEFT JOIN odin.google_credentials gg "
            "ON gg.identity_id = id.id "
            "WHERE id.id='" + identity_id + "' "
            "AND (cred.identity_id IS NOT NULL "
            "OR fb.identity_id IS NOT NULL "
            "OR gg.identity_id IS NOT NULL);"
        );
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

        if (not jwt_body.has_key("iss")) {
            auto const jwt_iss = fostlib::coerce<fostlib::string>(jwt_body["iss"]);
            if (not jwt_iss.startswith(odin::c_app_namespace.value())) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__, "App namespace prefix does not match");
            }
            auto const app_id =
                    jwt_iss.substr(odin::c_app_namespace.value().code_points());
            fostlib::json app = odin::app::get_detail(cnx, std::move(app_id));
            if (app.isnull()) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__, "App not found");
            }
            auto const app_token = odin::c_jwt_secret.value() + app_id;
            return std::vector<f5::byte>(
                    app_token.data().begin(), app_token.data().end());
        } else {
            auto const app_token = odin::c_jwt_secret.value();
            return std::vector<f5::byte>(
                    app_token.data().begin(), app_token.data().end());
        }
    }

    fostlib::nullable<fostlib::jwt::token> load_jwt(
        fostlib::pg::connection &cnx,
        fostlib::string jwt_string) {
        auto const jwt = fostlib::jwt::token::load(
            jwt_string,
            [&cnx](fostlib::json h, fostlib::json b) {
                return load_key(cnx, h, b);
            });
        if (not jwt) {
            fostlib::log::debug(odin::c_odin)(
                    "detail", "JWT not valid")(
                    "jwt", jwt_string);
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "JWT not valid");
        }
        return jwt;
    }

    fostlib::string get_identity_id_from_jwt(
        fostlib::pg::connection &cnx,
        fostlib::nullable<fostlib::jwt::token> jwt
    ) {
        if (jwt.value().payload.has_key("iss")) {
            auto const app_id = fostlib::coerce<fostlib::string>(
                                    jwt.value().payload["iss"])
                                    .substr(odin::c_app_namespace.value()
                                                    .code_points());
            auto const app_user_id = fostlib::coerce<fostlib::string>(
                                        jwt.value().payload["sub"]);
            auto const identity_id_set = cnx.exec(
                    "SELECT identity_id FROM "
                    "odin.app_user "
                    "WHERE app_id='"
                    + app_id + "'AND app_user_id='" + app_user_id
                    + "';");

            auto row = identity_id_set.begin();
            if (row == identity_id_set.end()) {
                return fostlib::string{};
            }
            return fostlib::coerce<fostlib::string>((*row)[0]);
        }
        return fostlib::coerce<fostlib::string>(
                jwt.value().payload["sub"]);
    }

    const class link_account : public fostlib::urlhandler::view {
      public:
        link_account() : view("odin.link.account") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {

            if (req.method() != "POST") {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "App Login required POST, this should be a 405");
            }

            auto const body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            auto const body = fostlib::json::parse(body_str);

            if (not body.has_key("from_account") || not body.has_key("to_account")) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__, "Must pass from_account and to_account field");
            }
            fostlib::pg::connection cnx(config);

            auto const from_account_jwt = load_jwt(cnx, fostlib::coerce<fostlib::string>(body["from_account"]));
            auto const to_account_jwt = load_jwt(cnx, fostlib::coerce<fostlib::string>(body["to_account"]));

            auto const from_identity_id = get_identity_id_from_jwt(cnx, from_account_jwt);
            auto const to_identity_id = get_identity_id_from_jwt(cnx, to_account_jwt);

            if (has_account_merged(cnx, from_identity_id)) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "The from_account has been merged.");
            }

            if (has_account_merged(cnx, to_identity_id)) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "The to_account has been merged.");
            }

            if (has_account_registerd(cnx, from_identity_id)) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "The from_account has been registerd.");
            }

            fostlib::json ret;
            fostlib::insert(ret, "message", "test");
            fostlib::mime::mime_headers headers;
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    fostlib::json::unparse(ret, true), headers, "application/json"));
            return std::make_pair(response, 422);
            }

    } c_link_account;
}

const fostlib::urlhandler::view &odin::view::link_account =
        c_link_account;
