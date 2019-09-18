/**
    Copyright 2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */

#include <odin/app.hpp>
#include <odin/credentials.hpp>
#include <odin/odin.hpp>
#include <odin/nonce.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>
#include <fost/insert>
#include <fost/json>
#include <fost/log>
#include <fost/push_back>
#include <fostgres/sql.hpp>

namespace {


    std::optional<fostlib::string>
            bearer_jwt(fostlib::http::server::request const &req) {
        if (req.headers().exists("Authorization")) {
            auto parts = fostlib::partition(
                    req.headers()["Authorization"].value(), " ");
            if (parts.first == "Bearer" && parts.second) {
                return parts.second;
            } else {
                fostlib::log::warning(odin::c_odin)(
                        "", "Invalid Authorization scheme")(
                        "scheme", parts.first)("data", parts.second);
            }
        }
        return {};
    }

    fostlib::json get_app_user(
            fostlib::pg::connection &cnx, const f5::u8view app_id, const f5::u8view app_user_id) {
        static const f5::u8view sql(
                "SELECT "
                "* "
                "FROM odin.app_user "
                "WHERE odin.app_user.app_id = $1 AND odin.app_user.app_user_id = $2; ");

        auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{app_id, app_user_id});
        auto &rs = data.second;
        auto row = rs.begin();
        if (row == rs.end()) {
            fostlib::log::warning(odin::c_odin)("", "App not found")("app_id", app_id)("app_user_id", app_user_id);
            return fostlib::json();
        }
        auto record = *row;
        fostlib::json app;
        for (std::size_t index{0}; index < record.size(); ++index) {
            const auto parts = fostlib::split(data.first[index], "__");
            // if (parts.size() && parts[parts.size() - 1] == "tableoid") continue;
            fostlib::jcursor pos;
            for (const auto &p : parts) pos /= p;
            fostlib::insert(app, pos, record[index]);
        }
        return app;
    }

    const class app_user : public fostlib::urlhandler::view {
      public:
        app_user() : view("odin.app.user") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {

            if (not req.headers().exists("__user")) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "The odin.app_user view must be wrapped by an "
                        "odin.app.secure or odin.secure"
                        "view on the secure path so that there is a valid JWT "
                        "to find App ID and User ID in");
            }


            if (req.method() != "PUT") {
                fostlib::json config;
                fostlib::insert(config, "view", "fost.response.405");
                fostlib::push_back(config, "configuration", "allow", "PUT");
                return execute(config, path, req, host);
            }

            auto parameters = fostlib::split(path, '/');
            if (parameters.size() != 2) {
                boost::shared_ptr<fostlib::mime> response(
                    new fostlib::text_body(fostlib::string("400 Bad Request\n"),
                        fostlib::mime::mime_headers(),
                        L"text/plain"));
                return std::make_pair(response, 400);
            }
            auto app_id = parameters[0];
            auto app_user_id = parameters[1];
            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            auto app_user = get_app_user(cnx, parameters[0], parameters[1]);
            if (app_user.isnull()) {
                auto identity_id = odin::reference();
                odin::create_user(cnx, identity_id);
                fostlib::json new_app_user;
                fostlib::insert(new_app_user, "app_id", app_id);
                fostlib::insert(new_app_user, "app_user_id", app_user_id);
                fostlib::insert(new_app_user, "identity_id", identity_id);
                fostlib::insert(new_app_user, "reference", odin::reference());
                cnx.insert("odin.app_user_app_user_id_ledger", new_app_user);
                cnx.commit();
            }

            auto const jwt_body = bearer_jwt(req);
            // Check whether it is APP JWT or non APP JWT
            fostlib::string secret;
            if (req.headers().exists("__app")) {
                auto const app_id = req.headers()["__app"].value();
                secret = odin::c_jwt_secret.value() + app_id;
            } else {
                secret = odin::c_jwt_secret.value();
            }

            auto const new_jwt =
                    odin::renew_jwt(jwt_body.value(), secret, config);
            auto const token = new_jwt.first;
            auto const exp = new_jwt.second;

            fostlib::mime::mime_headers headers;
            headers.add(
                    "Expires",
                    fostlib::coerce<fostlib::rfc1123_timestamp>(exp)
                            .underlying()
                            .underlying());

            boost::shared_ptr<fostlib::mime> response(
                    new fostlib::text_body(token, headers, L"application/jwt"));

            return std::make_pair(response, 200);
        }

    } c_app_user;
}

const fostlib::urlhandler::view &odin::view::app_user = c_app_user;