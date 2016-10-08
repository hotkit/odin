/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
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
        secure()
        : view("odin.secure") {
        }

        bool check_logout_claim(
            const fostlib::json &config, fostlib::http::server::request &req,
            const fostlib::jwt::token &jwt
        ) const {
            const auto &&claim = odin::c_jwt_logout_claim.value();
            if ( jwt.payload.has_key(claim) ) {
                fostlib::pg::connection cnx{fostgres::connection(config, req)};
                static const fostlib::string sql("SELECT "
                        "odin.identity.expires < now() AS expired, "
                        "odin.credentials.logout_count AS logout_count, "
                        "odin.identity.expires "
                    "FROM odin.credentials "
                    "JOIN odin.identity ON odin.identity.id=odin.credentials.identity_id "
                    "WHERE odin.identity.id = $1");
                auto data = fostgres::sql(cnx, sql, std::vector<fostlib::json>{jwt.payload["sub"]});
                auto &rs = data.second;
                auto row = rs.begin();
                if ( row == rs.end() ) {
                    fostlib::log::warning(odin::c_odin)
                        ("", "User not found")
                        ("username", jwt.payload["sub"]);
                    return false;
                }
                auto record = *row;
                if ( fostlib::coerce<bool>(record[0u]) ) {
                    fostlib::log::warning(odin::c_odin)
                        ("", "User account has expired")
                        ("username", jwt.payload["sub"])
                        ("expired", record[0u])
                        ("expires", record[2u]);
                    return false;
                }
                if ( record[1u] != jwt.payload[claim] ) {
                    fostlib::log::warning(odin::c_odin)
                        ("", "User account logout_count mismatch")
                        ("username", jwt.payload["sub"])
                        ("logout_count", record[1u])
                        ("jwt", jwt.payload);
                    return false;
                }
            }
            return true;
        }

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator () (
            const fostlib::json &config, const fostlib::string &path,
            fostlib::http::server::request &req,
            const fostlib::host &host
        ) const {
            // Set the reference header
            auto ref = odin::reference();
            req.headers().set("__odin_reference", ref);
            // Now check which sub-view to enter
            if ( req.headers().exists("Authorization") ) {
                auto parts = fostlib::partition(req.headers()["Authorization"].value(), " ");
                if ( parts.first == "Bearer" && not parts.second.isnull() ) {
                    auto jwt = fostlib::jwt::token::load(
                        odin::c_jwt_secret.value(), parts.second.value());
                    if ( not jwt.isnull() && check_logout_claim(config, req, jwt.value()) ) {
                        fostlib::log::debug(odin::c_odin)
                            ("", "JWT authenticated")
                            ("header", jwt.value().header)
                            ("payload", jwt.value().payload);
                        req.headers().set("__jwt", jwt.value().payload, "sub");
                        req.headers().set("__user",
                            fostlib::coerce<fostlib::string>(jwt.value().payload["sub"]));
                        return execute(config["secure"], path, req, host);
                    }
                } else {
                    fostlib::log::warning(odin::c_odin)
                        ("", "Invalid Authorization scheme")
                        ("scheme", parts.first)
                        ("data", parts.second);
                }
            }
            return execute(config["unsecure"], path, req, host);
        }
    } c_secure;


}


const fostlib::urlhandler::view &odin::view::secure = c_secure;

