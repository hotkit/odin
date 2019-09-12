/**
    Copyright 2018-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <odin/app.hpp>
#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/views.hpp>

#include <fost/crypto>
#include <fost/insert>
#include <fost/log>
#include <fostgres/sql.hpp>


namespace {


    std::vector<f5::byte> load_key(
            fostlib::pg::connection &cnx,
            fostlib::json jwt_header,
            fostlib::json jwt_body) {
        if (not jwt_body.has_key("iss")) {
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "No issuer in the JWT");
        }
        auto const jwt_iss = fostlib::coerce<fostlib::string>(jwt_body["iss"]);
        if (jwt_iss.find(odin::c_app_namespace.value()) == std::string::npos) {
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "App namespace prefix does not match");
        }
        auto const app_id =
                jwt_iss.substr(odin::c_app_namespace.value().code_points());
        fostlib::json app = odin::app::get_detail(cnx, std::move(app_id));
        if (app.isnull()) {
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "App not found");
        }
        auto const app_token = odin::c_jwt_secret.value() + app_id;
        return std::vector<f5::byte>(
                app_token.data().begin(), app_token.data().end());
    }


    /**
     * Base class used to handle the JWT code. It assumes that there are
     * a variety of ways of getting at the JWT value itself.
     */
    class jwt_secure : public fostlib::urlhandler::view {
      public:
        jwt_secure(f5::u8view n) : view(n) {}

        /// Must be implemented to load the actual JWT itself
        virtual std::optional<fostlib::string> find_jwt(
                const fostlib::json &config,
                fostlib::http::server::request const &req) const = 0;

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            auto const jwt_body = find_jwt(config, req);
            if (not config.has_key("secure")
                || not config.has_key("unsecure")) {
                throw fostlib::exceptions::not_implemented{
                        __PRETTY_FUNCTION__,
                        "Configuration must contain both `secure` and "
                        "`unsecure` views"};
            }
            if (jwt_body) {
                fostlib::pg::connection cnx{fostgres::connection(config, req)};
                auto jwt = fostlib::jwt::token::load(
                        jwt_body.value(),
                        [&cnx](fostlib::json h, fostlib::json b) {
                            return load_key(cnx, h, b);
                        });
                if (jwt) {
                    auto iss = fostlib::coerce<fostlib::string>(
                            jwt.value().payload["iss"]);
                    fostlib::log::debug(odin::c_odin)("", "JWT authenticated")(
                            "header",
                            jwt.value().header)("payload", jwt.value().payload);
                    if (jwt.value().payload.has_key("sub")) {
                        req.headers().set("__jwt", jwt.value().payload, "sub");
                        req.headers().set(
                                "__user",
                                fostlib::coerce<fostlib::string>(
                                        jwt.value().payload["sub"]));
                    }
                    req.headers().set(
                            "__app",
                            fostlib::coerce<fostlib::string>(
                                    jwt.value().payload["iss"])
                                    .substr(odin::c_app_namespace.value()
                                                    .code_points()));
                    return execute(config["secure"], path, req, host);
                }
            }
            return execute(config["unsecure"], path, req, host);
        }
    };


    const class app_secure : public jwt_secure {
      public:
        app_secure() : jwt_secure("odin.app.secure") {}

        std::optional<fostlib::string> find_jwt(
                const fostlib::json &,
                fostlib::http::server::request const &req) const {
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
    } c_app_secure;


    const class app_secure_cookie : public jwt_secure {
      public:
        app_secure_cookie() : jwt_secure("odin.app.secure.cookie") {}

        std::optional<fostlib::string> find_jwt(
                const fostlib::json &config,
                fostlib::http::server::request const &req) const {
            auto cookies = req.headers()["Cookie"];
            fostlib::parse_cookies(cookies);
            if (not config.has_key("cookie")) {
                throw fostlib::exceptions::not_implemented{
                        __PRETTY_FUNCTION__,
                        "Configuration item 'cookie' must be specified"};
            }
            return cookies.subvalue(
                    fostlib::coerce<f5::u8view>(config["cookie"]));
        }
    } c_app_secure_cookie;


}


const fostlib::urlhandler::view &odin::view::app_secure = c_app_secure;
