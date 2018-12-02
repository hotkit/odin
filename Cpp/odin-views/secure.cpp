/*
    Copyright 2016-2017 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/views.hpp>

#include <fost/crypto>
#include <fost/insert>
#include <fost/log>
#include <fostgres/sql.hpp>


namespace {


    const class secure : public fostlib::urlhandler::view {
      public:
        secure() : view("odin.secure") {}

        bool check_logout_claim(
                const fostlib::json &config,
                fostlib::http::server::request &req,
                const fostlib::jwt::token &jwt) const {
            const auto &&logout_claim = odin::c_jwt_logout_claim.value();
            const bool has_logout_claim = jwt.payload.has_key(logout_claim);
            if (not odin::c_jwt_trust.value()) {
                fostlib::pg::connection cnx{fostgres::connection(config, req)};
                static const fostlib::string sql_with_logout(
                        "SELECT "
                        "odin.identity.expires <= now() AS expired, "
                        "odin.identity.expires, "
                        "odin.credentials.logout_count AS logout_count "
                        "FROM odin.credentials "
                        "JOIN odin.identity ON "
                        "odin.identity.id=odin.credentials.identity_id "
                        "WHERE odin.identity.id = $1");
                static const fostlib::string sql_without_logout(
                        "SELECT "
                        "odin.identity.expires <= now() AS expired, "
                        "odin.identity.expires "
                        "FROM odin.identity "
                        "WHERE odin.identity.id = $1");
                auto data = fostgres::sql(
                        cnx,
                        has_logout_claim ? sql_with_logout : sql_without_logout,
                        std::vector<fostlib::json>{jwt.payload["sub"]});
                auto &rs = data.second;
                auto row = rs.begin();
                if (row == rs.end()) {
                    fostlib::log::warning(odin::c_odin)("", "User not found")(
                            "username", jwt.payload["sub"]);
                    return false;
                }
                fostlib::log::warning(odin::c_odin)("", "User found in DB");
                auto record = *row;
                const bool expired = fostlib::coerce<bool>(record[0u]);
                const auto &expires = record[1u];
                if (expired) {
                    fostlib::log::warning(odin::c_odin)(
                            "", "User account has expired")(
                            "username", jwt.payload["sub"])("expired", expired)(
                            "expires", expires);
                    return false;
                } else {
                    fostlib::log::warning(odin::c_odin)("", "Expiry check")(
                            "expired", expired)("expires", expires);
                }
                if (has_logout_claim) {
                    if (not odin::c_jwt_logout_check.value()) return true;
                    const auto logout_count = record[2u];
                    if (logout_count != jwt.payload[logout_claim]) {
                        fostlib::log::warning(odin::c_odin)(
                                "", "User account logout_count mismatch")(
                                "username", jwt.payload["sub"])(
                                "logout_count",
                                logout_count)("jwt", jwt.payload);
                        return false;
                    }
                }
            }
            return true;
        }

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            // Set the reference header
            auto ref = odin::reference();
            req.headers().set("__odin_reference", ref);
            // Now check which sub-view to enter
            if (req.headers().exists("Authorization")) {
                auto parts = fostlib::partition(
                        req.headers()["Authorization"].value(), " ");
                if (parts.first == "Bearer" && parts.second) {
                    auto jwt = fostlib::jwt::token::load(
                            odin::c_jwt_secret.value(), parts.second.value());
                    if (jwt && check_logout_claim(config, req, jwt.value())) {
                        fostlib::log::debug(odin::c_odin)(
                                "", "JWT authenticated")(
                                "header", jwt.value().header)(
                                "payload", jwt.value().payload);
                        req.headers().set("__jwt", jwt.value().payload, "sub");
                        req.headers().set(
                                "__user",
                                fostlib::coerce<fostlib::string>(
                                        jwt.value().payload["sub"]));
                        return execute(config["secure"], path, req, host);
                    }
                } else {
                    fostlib::log::warning(odin::c_odin)(
                            "", "Invalid Authorization scheme")(
                            "scheme", parts.first)("data", parts.second);
                }
            }
            return execute(config["unsecure"], path, req, host);
        }
    } c_secure;


}


const fostlib::urlhandler::view &odin::view::secure = c_secure;
