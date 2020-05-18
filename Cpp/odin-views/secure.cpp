/**
    Copyright 2016-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/views.hpp>

#include <fost/crypto>
#include <fost/insert>
#include <fost/log>
#include <fostgres/sql.hpp>


namespace {


    class secure_base : public fostlib::urlhandler::view {
      protected:
        secure_base(f5::u8view n) : view{n} {}

        virtual std::optional<fostlib::string> find_jwt(
                fostlib::json const &config,
                fostlib::http::server::request const &req) const = 0;

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            auto logger = fostlib::log::debug(odin::c_odin);
            logger("", "JWT decoder view");
            logger("headers", req.headers());
            // Set the reference header
            auto ref = odin::reference();
            req.headers().set("__odin_reference", ref);
            logger("ref", ref);
            // Now check which sub-view to enter
            auto const jwt_string = find_jwt(config, req);
            if (jwt_string) {
                auto jwt = fostlib::jwt::token::load(
                        odin::c_jwt_secret.value(), jwt_string.value());
                if (jwt) {
                    if (check_logout_claim(config, req, jwt.value())) {
                        fostlib::log::debug(odin::c_odin)(
                                "", "JWT authenticated")(
                                "header", jwt.value().header)(
                                "payload", jwt.value().payload);
                        req.headers().set("__jwt", jwt.value().payload, "sub");
                        if (not jwt.value().payload.has_key("sub")) {
                            fostlib::log::warning(odin::c_odin)(
                                    "", "JWT doesn't contain `sub` clain")(
                                    "payload", jwt.value().payload);
                        }
                        req.headers().set(
                                "__user",
                                fostlib::coerce<fostlib::string>(
                                        jwt.value().payload["sub"]));
                        logger("execute", "secure");
                        return execute(config["secure"], path, req, host);
                    } else {
                        logger("detail", "Log out claim not valid");
                        logger("jwt", jwt_string.value());
                        logger("execute", "unsecure");
                        return execute(config["unsecure"], path, req, host);
                    }
                } else {
                    logger("detail", "JWT not valid");
                    logger("jwt", jwt_string.value());
                    logger("execute", "unsecure");
                    return execute(config["unsecure"], path, req, host);
                }
            } else {
                logger("detail", "No JWT string found");
                logger("execute", "unsecure");
                return execute(config["unsecure"], path, req, host);
            }
        }

      private:
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
    };


    const struct secure : public secure_base {
        secure() : secure_base{"odin.secure"} {}

        std::optional<fostlib::string> find_jwt(
                fostlib::json const &config,
                fostlib::http::server::request const &req) const override {
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
            } else {
                fostlib::log::debug(odin::c_odin)("", "odin.secure")(
                        "failed", "No `Authorization` header");
                ;
            }
            return {};
        }
    } c_secure;


    const struct secure_cookie : public secure_base {
        secure_cookie() : secure_base{"odin.secure.cookie"} {}

        std::optional<fostlib::string> find_jwt(
                fostlib::json const &config,
                fostlib::http::server::request const &req) const override {
            auto cookies = req.headers()["Cookie"];
            fostlib::parse_cookies(cookies);
            if (not config.has_key("cookie")) {
                throw fostlib::exceptions::not_implemented{
                        __PRETTY_FUNCTION__,
                        "Configuration item 'cookie' must be specified"};
            }
            fostlib::log::debug(odin::c_odin)("", "Looking for JWT in cookies")(
                    "cookies", cookies);
            return cookies.subvalue(
                    fostlib::coerce<f5::u8view>(config["cookie"]));
        }
    } c_secure_cookie;


}


const fostlib::urlhandler::view &odin::view::secure = c_secure;
