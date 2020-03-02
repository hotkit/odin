/**
    Copyright 2018-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */

#include <odin/credentials.hpp>
#include <odin/google.hpp>
#include <odin/facebook.hpp>
#include <odin/odin.hpp>
#include <odin/nonce.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <fost/exception/parse_error.hpp>
#include <fost/insert>
#include <fost/log>
#include <fost/mailbox>
#include <fostgres/sql.hpp>


namespace {

    const fostlib::module c_odin_google(odin::c_odin, "google.cpp");

    std::pair<boost::shared_ptr<fostlib::mime>, int>
            respond(fostlib::string message, int code = 403) {
        fostlib::json ret;
        if (not message.empty())
            fostlib::insert(ret, "message", std::move(message));
        fostlib::mime::mime_headers headers;
        boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                fostlib::json::unparse(ret, true), headers, "application/json"));
        return std::make_pair(response, code);
    }

    const class google : public fostlib::urlhandler::view {
      public:
        google() : view("odin.google.login") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            if (req.method() != "POST")
                throw fostlib::exceptions::not_implemented(
                        __func__,
                        "Google login requires POST. This should be a 405");

            auto body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            fostlib::json body = fostlib::json::parse(body_str);
            if (!body.has_key("access_token"))
                throw fostlib::exceptions::not_implemented(
                        "odin.google.login", "Must pass access_token field");
            const auto access_token =
                    fostlib::coerce<fostlib::string>(body["access_token"]);
            fostlib::json user_detail;
            if (config.has_key("google-mock")) {
                if (fostlib::coerce<fostlib::string>(config["google-mock"])
                    == "OK") {
                    // Use access token as google ID
                    fostlib::insert(user_detail, "sub", access_token);
                    fostlib::insert(user_detail, "name", "Test User");
                    fostlib::insert(
                            user_detail, "email",
                            access_token + "@example.com");
                }
            } else {
                user_detail = odin::google::get_user_detail(access_token);
            }
            if (user_detail.isnull())
                throw fostlib::exceptions::not_implemented(
                        "odin.google.login", "User not authenticated");
            const auto google_user_id =
                    fostlib::coerce<f5::u8view>(user_detail["sub"]);
            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            const auto reference = odin::reference();
            auto google_user = odin::google::credentials(cnx, google_user_id);
            auto identity_id = reference;
            if (google_user.isnull()) {
                if (user_detail.has_key("email")) {
                    auto const email_owner_id =
                            odin::facebook::email_owner_identity_id(
                                    cnx,
                                    fostlib::coerce<fostlib::string>(
                                            user_detail["email"]));
                    if (email_owner_id.has_value()) {
                        identity_id = email_owner_id.value();
                    } else {
                        odin::create_user(cnx, identity_id);
                    }
                    odin::set_email(
                            cnx, reference, identity_id,
                            fostlib::coerce<fostlib::email_address>(
                                    user_detail["email"]));
                }
                if (user_detail.has_key("name")) {
                    const auto google_user_name =
                            fostlib::coerce<f5::u8view>(user_detail["name"]);
                    odin::set_full_name(
                            cnx, reference, identity_id, google_user_name);
                }
            } else {
                const fostlib::jcursor id("identity", "id");
                identity_id = fostlib::coerce<fostlib::string>(google_user[id]);
            }
            odin::google::set_google_credentials(
                    cnx, reference, identity_id, google_user_id);
            google_user = odin::google::credentials(cnx, google_user_id);

            if (body.has_key("installation_id")) {
                if (body["installation_id"].isnull()) {
                    throw fostlib::exceptions::not_implemented(
                            "odin.login", "Installation_id cannot be null");
                }
                const fostlib::string installation_id =
                        fostlib::coerce<fostlib::string>(
                                body["installation_id"]);
                if (installation_id.empty()) {
                    throw fostlib::exceptions::not_implemented(
                            "odin.login", "Installation_id cannot be empty");
                }
                odin::set_installation_id(
                        cnx, odin::reference(), identity_id, installation_id);
            }
            cnx.commit();

            auto jwt(odin::mint_login_jwt(google_user));
            auto exp = jwt.expires(
                    fostlib::coerce<fostlib::timediff>(config["expires"]),
                    false);
            jwt.claim(
                    "google_user_id",
                    google_user["google_credentials"]["google_user_id"]);
            fostlib::mime::mime_headers headers;
            headers.add(
                    "Expires",
                    fostlib::coerce<fostlib::rfc1123_timestamp>(exp)
                            .underlying()
                            .underlying());
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    fostlib::utf8_string(
                            jwt.token(odin::c_jwt_secret.value().data())),
                    headers, L"application/jwt"));
            return std::make_pair(response, 200);
        }
    } c_google;


}


const fostlib::urlhandler::view &odin::view::google = c_google;
