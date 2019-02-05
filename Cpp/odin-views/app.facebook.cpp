/**
    Copyright 2019 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include <odin/app.hpp>
#include <odin/facebook.hpp>
#include <odin/nonce.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <fost/insert>
#include <fost/push_back>
#include <fostgres/sql.hpp>


namespace {


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
            if (req.method() != "POST") {
                fostlib::json config;
                fostlib::insert(config, "view", "fost.response.405");
                fostlib::push_back(config, "configuration", "allow", "POST");
                return execute(config, path, req, host);
            }
            if (not req.headers().exists("__app")
                || not req.headers().exists("__user")) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "The odin.app.installation view must be wrapped by an "
                        "odin.app.secure view on the secure path so that there "
                        "is a valid JWT to find the App ID in");
            }

            auto body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            fostlib::json body = fostlib::json::parse(body_str);

            if (not body.has_key("access_token"))
                throw fostlib::exceptions::not_implemented(
                        "odin.app.facebook.login",
                        "Must pass access_token field");
            auto const access_token =
                    fostlib::coerce<fostlib::string>(body["access_token"]);
            if (not body.has_key("installation_id"))
                throw fostlib::exceptions::not_implemented(
                        "odin.app.facebook.login",
                        "Must pass installation_id field");
            auto const installation_id =
                    fostlib::coerce<fostlib::string>(body["installation_id"]);

            fostlib::json user_detail;
            if (config.has_key("facebook-mock")) {
                if (fostlib::coerce<fostlib::string>(config["facebook-mock"])
                    == "OK") {
                    // Use access token as facebook ID
                    fostlib::insert(user_detail, "id", access_token);
                    fostlib::insert(user_detail, "name", "Test User");
                    fostlib::insert(
                            user_detail, "email",
                            access_token + "@example.com");
                }
            } else {
                user_detail = odin::facebook::get_user_detail(access_token);
            }
            if (user_detail.isnull())
                throw fostlib::exceptions::not_implemented(
                        "odin.facebook.login", "User not authenticated");

            auto const facebook_user_id =
                    fostlib::coerce<f5::u8view>(user_detail["id"]);
            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            auto const reference = odin::reference();
            auto facebook_user =
                    odin::facebook::credentials(cnx, facebook_user_id);
            fostlib::string identity_id;
            if (facebook_user.isnull()) {
                identity_id = req.headers()["__user"].value();
                if (user_detail.has_key("name")) {
                    auto const facebook_user_name =
                            fostlib::coerce<f5::u8view>(user_detail["name"]);
                    odin::set_full_name(
                            cnx, reference, identity_id, facebook_user_name);
                }
                if (user_detail.has_key("email")) {
                    auto const facebook_user_email =
                            fostlib::coerce<fostlib::email_address>(
                                    user_detail["email"]);
                    if (odin::does_email_exist(
                                cnx,
                                fostlib::coerce<fostlib::string>(
                                        user_detail["email"]))) {
                        return respond("This email already exists", 422);
                    }
                    odin::set_email(
                            cnx, reference, identity_id, facebook_user_email);
                }
            } else {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__, "Already exists");
            }
            odin::facebook::set_facebook_credentials(
                    cnx, reference, identity_id, facebook_user_id);

            cnx.commit();

            auto jwt = odin::app::mint_user_jwt(
                    identity_id, req.headers()["__app"].value(),
                    fostlib::coerce<fostlib::timediff>(config["expires"]));
            fostlib::mime::mime_headers headers;
            headers.add(
                    "Expires",
                    fostlib::coerce<fostlib::rfc1123_timestamp>(jwt.second)
                            .underlying()
                            .underlying()
                            .c_str());
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    jwt.first, headers, L"application/jwt"));

            return std::make_pair(response, 200);
        }
    } c_fb_login;


}
