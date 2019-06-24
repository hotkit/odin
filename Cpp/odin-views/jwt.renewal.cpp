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

#include <fostgres/sql.hpp>

#include <fost/insert>
#include <fost/json>
#include <fost/push_back>
#include <fost/string.hpp>

namespace {

    const class jwt_renewal : public fostlib::urlhandler::view {
      public:
        jwt_renewal() : view("odin.jwt.renewal") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {

            if (!req.headers().exists("__app") && !req.headers().exists("__user")) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "The odin.jwt.renewal view must be wrapped by an "
                        "odin.app.secure "
                        "view on the secure path so that there is a valid JWT "
                        "to find App ID and User ID in");
            }

            if (req.method() != "GET") {
                fostlib::json config;
                fostlib::insert(config, "view", "fost.response.405");
                fostlib::push_back(config, "configuration", "allow", "GET");
                return execute(config, path, req, host);
            }

            auto const app_id = req.headers()["__app"].value();
            auto const jwt_user = req.headers()["__user"].value();
            
            // Check whether it is APP JWT or non APP JWT

            fostlib::timestamp exp;
            fostlib::utf8_string token;
            if (req.headers().exists("__app")) {
                
                auto jwt = odin::app::mint_user_jwt(
                        jwt_user, app_id,
                        fostlib::coerce<fostlib::timediff>(config["expires"]));
                token = jwt.first;
                exp = jwt.second;
            } else {
                
                fostlib::pg::connection cnx{fostgres::connection(config, req)};
                
                static const fostlib::string sql("SELECT * FROM odin.identity WHERE odin.identity.id = $1");
                auto data = fostgres::sql(
                    cnx, sql, std::vector<fostlib::string>{jwt_user});
                auto &rs = data.second;
                auto row = rs.begin();
                auto record = *row;

                fostlib::json user;
                for (std::size_t index{0}; index < record.size(); ++index) {
                    const auto parts = fostlib::split(data.first[index], "__");
                    fostlib::jcursor pos;
                    for (const auto &p : parts) pos /= p;
                    fostlib::insert(user, pos, record[index]);
                    
                }

                std::map<fostlib::string, fostlib::json> format;
                format.insert(std::pair<fostlib::string, fostlib::json>("identity", user));
                fostlib::json identity_user = format;

                auto jwt_response(odin::mint_login_jwt(identity_user));
                exp = jwt_response.expires(
                    fostlib::coerce<fostlib::timediff>(config["expires"]),
                    false);
                token = jwt_response.token(odin::c_jwt_secret.value().data());

            }

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

const fostlib::urlhandler::view &odin::view::jwt_renewal = c_jwt_renewal;
