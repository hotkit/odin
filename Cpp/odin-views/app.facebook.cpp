/**
    Copyright 2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <odin/app.hpp>
#include <odin/facebook.hpp>
#include <odin/nonce.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <fost/insert>
#include <fost/log>
#include <fost/push_back>
#include <fostgres/sql.hpp>

#include <pqxx/except>


namespace {


    fostlib::module const c_odin_app_facebook{odin::c_odin_app, "facebook"};


    inline std::pair<boost::shared_ptr<fostlib::mime>, int>
            respond(fostlib::string message, int code = 403) {
        fostlib::json ret;
        if (not message.empty())
            fostlib::insert(ret, "message", std::move(message));
        fostlib::mime::mime_headers headers;
        boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                fostlib::json::unparse(ret, true), headers, "application/json"));
        return std::make_pair(response, code);
    }


    const class fb_login : public fostlib::urlhandler::view {
      public:
        fb_login() : view("odin.app.facebook.login") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            auto logger{fostlib::log::debug(c_odin_app_facebook)};
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
                        "The odin.app.facebook.login view must be wrapped by "
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

            if (not body.has_key("access_token")) 
                throw fostlib::exceptions::not_implemented(
                        "odin.app.facebook.login",
                        "Must pass access_token field");
            auto const access_token =
                    fostlib::coerce<fostlib::string>(body["access_token"]);

            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            fostlib::json user_detail;
            user_detail =
                    odin::facebook::get_user_detail(cnx, access_token, config);
            logger("user_detail", user_detail);
            if (user_detail.isnull()) 
                throw fostlib::exceptions::not_implemented(
                        "odin.app.facebook.login", "User not authenticated");

            auto const facebook_user_id =
                    fostlib::coerce<f5::u8view>(user_detail["id"]);
            auto const reference = odin::reference();
            f5::u8view const app_id = req.headers()["__app"].value();
            auto facebook_user = odin::facebook::app_credentials(
                    cnx, facebook_user_id, app_id);
            logger("facebook_user", facebook_user);
            fostlib::string identity_id;
            fostlib::string app_user_id;
            if (facebook_user.isnull()) {
                /// We've never seen this Facebook identity before,
                /// we take as a new user registration. If this is a new
                /// installation then this is a new user registration, if
                /// the JWT represents an existing user then this links
                /// their Facebook a/c to their pre-existing identity.
                identity_id = req.headers()["__user"].value();
                app_user_id = req.headers()["__app_user"].value();
                if (user_detail.has_key("email")) {
                    auto const email_owner_id = odin::email_owner_id(cnx, fostlib::coerce<fostlib::string>(user_detail["email"]));
                    if (email_owner_id.has_value()) {
                        fostlib::json merge_annotation;
                        fostlib::insert(merge_annotation, "app", req.headers()["__app"]);
                        try {
                            odin::link_account(cnx, req.headers()["__user"].value(), email_owner_id.value(), merge_annotation);   
                        } catch (const pqxx::unique_violation &e) {
                            /// We replace the identity with the new one -- case 2 above
                        } catch (...) { throw; }
                        identity_id = fostlib::coerce<fostlib::string>(email_owner_id.value());    
                    } else {
                        odin::set_email(
                            cnx, reference, identity_id, fostlib::coerce<fostlib::email_address>(user_detail["email"]));
                    }
                } 
                if (user_detail.has_key("name")) {
                    auto const facebook_user_name =
                            fostlib::coerce<f5::u8view>(user_detail["name"]);
                    odin::set_full_name(
                            cnx, reference, identity_id, facebook_user_name);
                }
                odin::facebook::set_facebook_credentials(
                        cnx, reference, identity_id, facebook_user_id);

                cnx.commit();
            } else if (
                    facebook_user["identity"]["id"]
                    == req.headers()["__user"].value()) {
                /// An existing user has logging in on a same device.
                app_user_id = fostlib::coerce<fostlib::string>(
                        facebook_user["app_user"]["app_id"]);
            } else {
                /// An existing user has logged in to a new device. Probably
                /// there are two cases here:
                /// 1. The user doesn't already have an account on this app.
                /// 2. The user does already have an account on this app.
                identity_id = fostlib::coerce<fostlib::string>(
                        facebook_user["identity"]["id"]);
                fostlib::json merge_annotation;
                fostlib::insert(merge_annotation, "app", req.headers()["__app"]);
                
                try {
                    /// Case 1 above
                    odin::link_account(cnx, req.headers()["__user"].value(), fostlib::coerce<f5::u8view>(facebook_user["identity"]["id"]), merge_annotation);
                    cnx.commit();
                    auto facebook_user = odin::facebook::app_credentials(
                            cnx, facebook_user_id, app_id);
                    app_user_id = fostlib::coerce<fostlib::string>(
                            facebook_user["app_user"]["app_user_id"]);
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
                    jwt.first, headers, L"application/jwt"));

            return std::make_pair(response, 200);
        }
    } c_fb_login;


}
