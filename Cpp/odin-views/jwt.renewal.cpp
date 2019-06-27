/**
    Copyright 2019 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/

#include <odin/app.hpp>
#include <odin/credentials.hpp>
#include <odin/odin.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <fost/insert>
#include <fost/json>
#include <fost/log>
#include <fost/push_back>

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
    
    const class jwt_renewal : public fostlib::urlhandler::view {
      public:
        jwt_renewal() : view("odin.jwt.renewal") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {

            if (not req.headers().exists("__user")) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "The odin.jwt.renewal view must be wrapped by an "
                        "odin.app.secure or odin.secure"
                        "view on the secure path so that there is a valid JWT "
                        "to find App ID and User ID in");
            }

            if (req.method() != "GET") {
                fostlib::json config;
                fostlib::insert(config, "view", "fost.response.405");
                fostlib::push_back(config, "configuration", "allow", "GET");
                return execute(config, path, req, host);
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
    
            auto const new_jwt = odin::renew_jwt(jwt_body.value(), secret, config);
            auto const token = new_jwt.first;
            auto const exp = new_jwt.second;

            fostlib::mime::mime_headers headers;
            headers.add(
                        "Expires",
                        fostlib::coerce<fostlib::rfc1123_timestamp>(exp)
                                .underlying()
                                .underlying());

            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                        token, headers, L"application/jwt"));

            return std::make_pair(response, 200);
        }

    } c_jwt_renewal;
}
