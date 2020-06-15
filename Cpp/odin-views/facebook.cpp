/**
    Copyright 2016-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */

#include <odin/credentials.hpp>
#include <odin/facebook.hpp>
#include <odin/odin.hpp>
#include <odin/nonce.hpp>
#include <odin/thirdparty.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <fost/exception/parse_error.hpp>
#include <fost/insert>
#include <fost/log>
#include <fost/push_back>
#include <fost/mailbox>
#include <fostgres/sql.hpp>


namespace {

    const fostlib::module c_odin_facebook(odin::c_odin, "facebook.cpp");

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

    const class facebook : public fostlib::urlhandler::view {
      public:
        facebook() : view("odin.facebook.login") {}

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

            auto body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            fostlib::json body = fostlib::json::parse(body_str);
            if (!body.has_key("access_token"))
                throw fostlib::exceptions::not_implemented(
                        "odin.facebook.login", "Must pass access_token field");
            const auto access_token =
                    fostlib::coerce<fostlib::string>(body["access_token"]);
            fostlib::json user_detail;
            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            user_detail =
                    odin::facebook::get_user_detail(cnx, access_token, config);
            if (user_detail.isnull())
                throw fostlib::exceptions::not_implemented(
                        "odin.facebook.login", "User not authenticated");
            const auto facebook_user_id =
                    fostlib::coerce<f5::u8view>(user_detail["id"]);
            const auto reference = odin::reference();
            auto facebook_user =
                    odin::facebook::credentials(cnx, facebook_user_id);
            auto identity_id = reference;
            if (facebook_user.isnull()) {
                if (user_detail.has_key("email")) {
                    auto const email_owner_id =
                            odin::thirdparty::email_owner_identity_id(
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
                    const auto facebook_user_name =
                            fostlib::coerce<f5::u8view>(user_detail["name"]);
                    odin::set_full_name(
                            cnx, reference, identity_id, facebook_user_name);
                }
            } else {
                const fostlib::jcursor id("identity", "id");
                identity_id =
                        fostlib::coerce<fostlib::string>(facebook_user[id]);
            }
            odin::facebook::set_facebook_credentials(
                    cnx, reference, identity_id, facebook_user_id);

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

            facebook_user = odin::facebook::credentials(cnx, facebook_user_id);

            auto jwt(odin::mint_login_jwt(facebook_user));
            auto exp = jwt.expires(
                    fostlib::coerce<fostlib::timediff>(config["expires"]),
                    false);
            jwt.claim(
                    "facebook_user_id",
                    facebook_user["facebook_credentials"]["facebook_user_id"]);
            fostlib::mime::mime_headers headers;
            headers.add(
                    "Expires",
                    fostlib::coerce<fostlib::rfc1123_timestamp>(exp)
                            .underlying()
                            .underlying());
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    fostlib::utf8_string(
                            jwt.token(odin::c_jwt_secret.value().data())),
                    headers, "application/jwt"));
            return std::make_pair(response, 200);
        }
    } c_facebook;


    const class facebook_link : public fostlib::urlhandler::view {
      public:
        facebook_link() : view("odin.facebook.link") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            if (req.method() != "PUT") {
                throw fostlib::exceptions::not_implemented(
                        __func__,
                        "Facebook link requires PUT. This should be a 405");
            }
            auto body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            fostlib::json body = fostlib::json::parse(body_str);
            if (!body.has_key("access_token")) {
                throw fostlib::exceptions::not_implemented(
                        "odin.facebook.link", "Must pass access_token field");
            }
            const auto access_token =
                    fostlib::coerce<fostlib::string>(body["access_token"]);

            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            fostlib::json user_detail;
            user_detail =
                    odin::facebook::get_user_detail(cnx, access_token, config);
            if (user_detail.isnull()) {
                throw fostlib::exceptions::not_implemented(
                        "odin.facebook.link", "User not authenticated");
            }
            if (!req.headers().exists("__user")) {
                throw fostlib::exceptions::not_implemented(
                        "odin.facebook.link",
                        "Request header must contain __user field");
            }
            auto identity_id =
                    fostlib::coerce<fostlib::string>(req.headers()["__user"]);
            const auto facebook_user_id =
                    fostlib::coerce<f5::u8view>(user_detail["id"]);
            auto facebook_user =
                    odin::facebook::credentials(cnx, facebook_user_id);

            if (!facebook_user.isnull()) {
                if (identity_id
                    == fostlib::coerce<fostlib::string>(
                            facebook_user["facebook_credentials"]
                                         ["identity_id"])) {
                    throw fostlib::exceptions::not_implemented(
                            "odin.facebook.link",
                            "This user already linked to this facebook");
                } else {
                    throw fostlib::exceptions::not_implemented(
                            "odin.facebook.link",
                            "Other user already linked with this facebook");
                }
            }
            const auto reference = odin::reference();
            odin::facebook::set_facebook_credentials(
                    cnx, reference, identity_id, facebook_user_id);
            cnx.commit();

            facebook_user = odin::facebook::credentials(cnx, facebook_user_id);

            auto jwt_response(odin::mint_login_jwt(facebook_user));
            auto exp = jwt_response.expires(
                    fostlib::coerce<fostlib::timediff>(config["expires"]),
                    false);
            jwt_response.claim(
                    "facebook_user_id",
                    facebook_user["facebook_credentials"]["facebook_user_id"]);
            fostlib::mime::mime_headers headers;
            headers.add(
                    "Expires",
                    fostlib::coerce<fostlib::rfc1123_timestamp>(exp)
                            .underlying()
                            .underlying());
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    fostlib::utf8_string(jwt_response.token(
                            odin::c_jwt_secret.value().data())),
                    headers, "application/jwt"));
            return std::make_pair(response, 202);
        }

    } c_facebook_link;


}


const fostlib::urlhandler::view &odin::view::facebook = c_facebook;
const fostlib::urlhandler::view &odin::view::facebook_link = c_facebook_link;
