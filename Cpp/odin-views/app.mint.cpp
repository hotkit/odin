/**
    Copyright 2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */

#include <odin/app.hpp>
#include <odin/credentials.hpp>
#include <odin/group.hpp>
#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <pqxx/except>
#include <fost/exception/parse_error.hpp>
#include <fost/insert>
#include <fost/json>
#include <fost/log>
#include <fostgres/sql.hpp>


namespace {

    fostlib::json parse_payload(fostlib::http::server::request &req) {
        // TODO: Support multiple ContentType
        auto body_str = fostlib::coerce<fostlib::string>(
                fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
        return fostlib::json::parse(body_str);
    }

    const class app_mint : public fostlib::urlhandler::view {
      public:
        app_mint() : view("odin.app.mint") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {

            if (!req.headers().exists("__app")) {
                throw fostlib::exceptions::not_implemented(
                        __func__,
                        "The odin.app.mint view must be wrapped by an "
                        "odin.app.secure "
                        "view on the secure path so that there is a valid JWT "
                        "to find App ID and User ID in");
            }

            if (req.method() != "POST")
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "Required POST, this should be a 405");

            auto const app_id = req.headers()["__app"].value();
            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            fostlib::json app = odin::app::get_detail(cnx, app_id);

            fostlib::json body = parse_payload(req);
            if (!body.has_key("username") || !body.has_key("password")) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "Must pass both username and password fields");
            }
            const auto username =
                    fostlib::coerce<fostlib::string>(body["username"]);
            const auto password =
                    fostlib::coerce<fostlib::string>(body["password"]);

            auto user = odin::app_credentials(
                    cnx, username, password, app_id, req.remote_address());
            if (user.isnull()) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__, "User not found");
            }
            const auto jwt_user = req.headers()["__user"].value();
            fostlib::string app_user_id;
            if (user["app_user"]["app_user_id"] != req.headers()["__app_user"].value()) {
                // identity_id mismatch, triggering merge account
                fostlib::json merge_value;
                fostlib::insert(merge_value, "from_identity_id", jwt_user);
                fostlib::insert(
                        merge_value, "to_identity_id", user["identity"]["id"]);
                try {
                    cnx.insert("odin.merge_ledger", merge_value);
                    cnx.commit();
                    auto user = odin::app_credentials(
                            cnx, username, password, app_id, req.remote_address());
                    app_user_id = fostlib::coerce<fostlib::string>(user["app_user"]["app_user_id"]);

                } catch (const pqxx::unique_violation &e) {
                    // Cannot merge, abandon the unregistered identity.
                    fostlib::log::info(odin::c_odin)(
                            "", "Merge account failed")("from", jwt_user)(
                            "to",
                            user["identity"]["id"])("abandoned", jwt_user);
                }
            } else {
                app_user_id = req.headers()["__app_user"].value();
            }
            auto const identity_id =
                    fostlib::coerce<f5::u8view>(user["identity"]["id"]);
            auto jwt = odin::app::mint_user_jwt(
                    app_user_id, app_id,
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

    } c_app_mint;


}


const fostlib::urlhandler::view &odin::view::app_mint = c_app_mint;
