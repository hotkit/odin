/**
    Copyright 2020 Red Anchor Trading Co. Ltd.
    
    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <odin/app.hpp>
#include <odin/apple.hpp>
#include <odin/google.hpp>
#include <odin/nonce.hpp>
#include <odin/thirdparty.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <fost/insert>
#include <fost/log>
#include <fost/push_back>
#include <fostgres/sql.hpp>

#include <pqxx/except>


namespace {


    fostlib::module const c_odin_app_apple{odin::c_odin_app, "apple"};

    const class apple_login : public fostlib::urlhandler::view {
      public:
        apple_login() : view("odin.app.apple.login") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            auto logger{fostlib::log::debug(c_odin_app_apple)};
            if (req.method() != "POST") {
                fostlib::json config;
                fostlib::insert(config, "view", "fost.response.405");
                fostlib::push_back(config, "configuration", "allow", "POST");
                return execute(config, path, req, host);
            }
            if (not req.headers().exists("__app")
                || not req.headers().exists("__user")
                || not req.headers().exists("__app_user")) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "The odin.app.apple.login view must be wrapped by "
                        "an odin.app.secure view on the secure path so that "
                        "there is a valid JWT to find the App ID in");
            }
            logger("__app", req.headers()["__app"]);
            logger("__user", req.headers()["__user"]);
            logger("__app_user", req.headers()["__app_user"]);

            auto body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            fostlib::json body = fostlib::json::parse(body_str);
            logger("body", body);

            if (not body.has_key("access_token") || not body.has_key("name"))
                throw fostlib::exceptions::not_implemented(
                        "odin.app.apple.login",
                        "Must pass access_token and name field");
            auto const access_token =
                    fostlib::coerce<fostlib::string>(body["access_token"]);

            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            fostlib::json user_detail;
            user_detail = odin::apple::get_user_detail(access_token);
            logger("user_detail", user_detail);
            if (user_detail.isnull())
                throw fostlib::exceptions::not_implemented(
                        "odin.app.apple.login", "User not authenticated");

            auto const apple_user_id =
                    fostlib::coerce<f5::u8view>(user_detail["user_id"]);
            auto const reference = odin::reference();
            f5::u8view const app_id = req.headers()["__app"].value();
            auto apple_user =
                    odin::apple::app_credentials(cnx, apple_user_id, app_id);
            logger("apple_user", apple_user);
            fostlib::string identity_id;
            fostlib::string app_user_id;
            if (apple_user.isnull()) {
                /// We've never seen this apple identity before,
                /// we take as a new user registration. If this is a new
                /// installation then this is a new user registration, if
                /// the JWT represents an existing user then this links
                /// their apple a/c to their pre-existing identity.
                identity_id = req.headers()["__user"].value();
                app_user_id = req.headers()["__app_user"].value();
                if (user_detail.has_key("email")) {
                    auto const email_owner_id =
                            odin::thirdparty::email_owner_identity_id(
                                    cnx,
                                    fostlib::coerce<fostlib::string>(
                                            user_detail["email"]));
                    if (email_owner_id.has_value()) {
                        fostlib::json merge_annotation;
                        fostlib::insert(
                                merge_annotation, "app",
                                req.headers()["__app"]);
                        try {
                            odin::link_account(
                                    cnx, req.headers()["__user"].value(),
                                    email_owner_id.value(), merge_annotation);
                        } catch (const pqxx::unique_violation &e) {
                            /// We replace the identity with the new one -- case
                            /// 2 above
                        } catch (...) { throw; }
                        identity_id = fostlib::coerce<fostlib::string>(
                                email_owner_id.value());
                        auto const apple_app_user = odin::app::get_app_user(
                                cnx, app_id, identity_id);
                        if (!apple_app_user.isnull()) {
                            app_user_id = fostlib::coerce<fostlib::string>(
                                    apple_app_user["app"]["app_user_id"]);
                        }
                    } else {
                        odin::set_email(
                                cnx, reference, identity_id,
                                fostlib::coerce<fostlib::email_address>(
                                        user_detail["email"]));
                    }
                }
                odin::set_full_name(
                        cnx, reference, identity_id,
                        fostlib::coerce<f5::u8view>(body["name"]));

                odin::apple::set_apple_credentials(
                        cnx, reference, identity_id, apple_user_id);

                cnx.commit();
            } else if (
                    apple_user["identity"]["id"]
                    == req.headers()["__user"].value()) {
                /// An existing user has logging in on a same device.
                app_user_id = fostlib::coerce<fostlib::string>(
                        apple_user["app_user"]["app_id"]);
            } else {
                /// An existing user has logged in to a new device. Probably
                /// there are two cases here:
                /// 1. The user doesn't already have an account on this app.
                /// 2. The user does already have an account on this app.
                identity_id = fostlib::coerce<fostlib::string>(
                        apple_user["identity"]["id"]);
                fostlib::json merge_annotation;
                fostlib::insert(
                        merge_annotation, "app", req.headers()["__app"]);

                try {
                    /// Case 1 above
                    odin::link_account(
                            cnx, req.headers()["__user"].value(),
                            fostlib::coerce<f5::u8view>(
                                    apple_user["identity"]["id"]),
                            merge_annotation);
                    cnx.commit();
                    auto apple_user = odin::apple::app_credentials(
                            cnx, apple_user_id, app_id);
                    app_user_id = fostlib::coerce<fostlib::string>(
                            apple_user["app_user"]["app_user_id"]);
                } catch (const pqxx::unique_violation &e) {
                    /// We replace the identity with the new one -- case 2 above
                } catch (...) { throw; }
            }

            auto jwt = odin::app::mint_user_jwt(
                    app_user_id, req.headers()["__app"].value(),
                    fostlib::coerce<fostlib::timediff>(config["expires"]));
            fostlib::mime::mime_headers headers;
            headers.add(
                    "Expires",
                    fostlib::coerce<fostlib::rfc1123_timestamp>(jwt.second)
                            .underlying()
                            .underlying());
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    jwt.first, headers, "application/jwt"));

            return std::make_pair(response, 200);
        }
    } c_apple_login;


}
