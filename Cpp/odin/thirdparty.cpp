/**
    Copyright 2018-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */

#include <odin/thirdparty.hpp>
#include <odin/app.hpp>
#include <odin/fg/native.hpp>
#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/user.hpp>

#include <fost/core>
#include <fost/log>
#include <fost/http>
#include <fost/postgres>
#include <fostgres/sql.hpp>
#include <fost/urlhandler>

#include <pqxx/except>


std::optional<f5::u8string> odin::thirdparty::email_owner_identity_id(
        fostlib::pg::connection &cnx, fostlib::string email) {
    const f5::u8string sql(
            "SELECT oi.id FROM odin.identity oi "
            "LEFT JOIN odin.facebook_credentials fc "
            "ON oi.id = fc.identity_id "
            "LEFT JOIN odin.google_credentials gg "
            "ON oi.id = gg.identity_id "
            "LEFT JOIN odin.apple_credentials ap "
            "ON oi.id = ap.identity_id "
            "WHERE email=$1 "
            "AND (fc.identity_id IS NOT NULL "
            "OR gg.identity_id IS NOT NULL "
            "OR ap.identity_id IS NOT NULL);");
    auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{email});
    auto &rs = data.second;
    auto row = rs.begin();
    if (row == rs.end()) { return {}; }
    if (++row != rs.end()) {
        fostlib::log::error(c_odin)("", "More than one email owner returned")(
                "email", email);
        return {};
    }
    return fostlib::coerce<f5::u8string>((*row)[std::size_t{0}]);
}


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


std::pair<boost::shared_ptr<fostlib::mime>, int> odin::thirdparty::login(
        const fostlib::json &config,
        const fostlib::string &path,
        fostlib::http::server::request &req,
        const fostlib::host &host,
        const fostlib::string &view_name,
        fostlib::log::detail::log_object &logger,
        const std::function<
                fostlib::json(fostlib::pg::connection &, f5::u8view)>
                &thirdparty_user_detail,
        const std::function<fostlib::json(
                fostlib::pg::connection &, const f5::u8view &, const f5::u8view &)>
                &thirdparty_app_credential,
        const std::function<void(
                fostlib::pg::connection &, f5::u8view, f5::u8view, f5::u8view)>
                &set_thirdparty_credentials) {
    if (req.method() != "POST") {
        fostlib::json config;
        fostlib::insert(config, "view", "fost.response.405");
        fostlib::push_back(config, "configuration", "allow", "POST");
        return fostlib::urlhandler::view::execute(config, path, req, host);
    }
    if (not req.headers().exists("__app") || not req.headers().exists("__user")
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
                "odin.app.facebook.login", "Must pass access_token field");
    auto const access_token =
            fostlib::coerce<fostlib::string>(body["access_token"]);

    fostlib::pg::connection cnx{fostgres::connection(config, req)};
    auto const user_detail = thirdparty_user_detail(cnx, access_token);
    logger("user_detail", user_detail);
    if (user_detail.isnull())
        throw fostlib::exceptions::not_implemented(
                view_name, "User not authenticated");

    auto const thirdparty_user_id =
            fostlib::coerce<f5::u8view>(user_detail["user_id"]);
    auto const reference = odin::reference();
    f5::u8view const app_id = req.headers()["__app"].value();
    auto thirdparty_user =
            thirdparty_app_credential(cnx, thirdparty_user_id, app_id);
    logger("thirdparty_user", thirdparty_user);
    fostlib::string identity_id;
    fostlib::string app_user_id;
    if (thirdparty_user.isnull()) {
        /// We've never seen this Facebook identity before,
        /// we take as a new user registration. If this is a new
        /// installation then this is a new user registration, if
        /// the JWT represents an existing user then this links
        /// their Facebook a/c to their pre-existing identity.
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
                        merge_annotation, "app", req.headers()["__app"]);
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
                auto const email_owner_app_user =
                        odin::app::get_app_user(cnx, app_id, identity_id);
                if (!email_owner_app_user.isnull()) {
                    app_user_id = fostlib::coerce<fostlib::string>(
                            email_owner_app_user["app"]["app_user_id"]);
                }
            } else {
                odin::set_email(
                        cnx, reference, identity_id,
                        fostlib::coerce<fostlib::email_address>(
                                user_detail["email"]));
            }
        }
        if (user_detail.has_key("name")) {
            auto const thirdparty_user_name =
                    fostlib::coerce<f5::u8view>(user_detail["name"]);
            odin::set_full_name(
                    cnx, reference, identity_id, thirdparty_user_name);
        }
        set_thirdparty_credentials(
                cnx, reference, identity_id, thirdparty_user_id);

        cnx.commit();
    } else if (
            thirdparty_user["identity"]["id"]
            == req.headers()["__user"].value()) {
        /// An existing user has logging in on a same device.
        app_user_id = fostlib::coerce<fostlib::string>(
                thirdparty_user["app_user"]["app_id"]);
    } else {
        /// An existing user has logged in to a new device. Probably
        /// there are two cases here:
        /// 1. The user doesn't already have an account on this app.
        /// 2. The user does already have an account on this app.
        identity_id = fostlib::coerce<fostlib::string>(
                thirdparty_user["identity"]["id"]);
        fostlib::json merge_annotation;
        fostlib::insert(merge_annotation, "app", req.headers()["__app"]);

        try {
            /// Case 1 above
            odin::link_account(
                    cnx, req.headers()["__user"].value(),
                    fostlib::coerce<f5::u8view>(
                            thirdparty_user["identity"]["id"]),
                    merge_annotation);
            cnx.commit();

            app_user_id = fostlib::coerce<fostlib::string>(
                    req.headers()["__app_user"].value());
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
    boost::shared_ptr<fostlib::mime> response(
            new fostlib::text_body(jwt.first, headers, "application/jwt"));

    return std::make_pair(response, 200);
}
