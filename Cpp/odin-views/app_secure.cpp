/*
    Copyright 2016-2017 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
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


    const class app_secure : public fostlib::urlhandler::view {
    public:
        app_secure()
        : view("odin.app.secure") {
        }

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator () (
            const fostlib::json &config, const fostlib::string &path,
            fostlib::http::server::request &req,
            const fostlib::host &host
        ) const {
            const auto paths = fostlib::split(path, "/");
            if ( paths.size() != 1)
                throw fostlib::exceptions::not_implemented(__func__,
                    "Must pass app_id in the URL");
            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            const auto app_id = paths[0];
            fostlib::json app = odin::app::get_detail(cnx, app_id);
            cnx.commit();
            if ( app.isnull() )
                throw fostlib::exceptions::not_implemented(__func__, "App not found");
            // Set the reference header
            auto ref = odin::reference();
            req.headers().set("__odin_reference", ref);
            // Now check which sub-view to enter
            if ( req.headers().exists("Authorization") ) {
                auto parts = fostlib::partition(req.headers()["Authorization"].value(), " ");
                if ( parts.first == "Bearer" && parts.second ) {
                    auto jwt = fostlib::jwt::token::load(
                        fostlib::coerce<fostlib::string>(app["app"]["token"]), parts.second.value());
                    if ( jwt ) {
                        auto iss = fostlib::coerce<fostlib::string>(jwt.value().payload["iss"]);
                        if ( iss == app_id ) {
                            fostlib::log::debug(odin::c_odin)
                                ("", "JWT authenticated")
                                ("header", jwt.value().header)
                                ("payload", jwt.value().payload);
                            req.headers().set("__jwt", jwt.value().payload, "sub");
                            req.headers().set("__app",
                                fostlib::coerce<fostlib::string>(jwt.value().payload["iss"]));
                            return execute(config["secure"], path, req, host);
                        }
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
    } c_app_secure;


}


const fostlib::urlhandler::view &odin::view::app_secure = c_app_secure;

