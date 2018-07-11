/*
    Copyright 2016-2017 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/credentials.hpp>
#include <odin/odin.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <fost/crypto>
#include <fost/insert>
#include <fostgres/sql.hpp>


namespace {


    const class logout : public fostlib::urlhandler::view {
    public:
        logout()
        : view("odin.logout") {
        }

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator () (
            const fostlib::json &config,
            const fostlib::string &path,
            fostlib::http::server::request &req,
            const fostlib::host &host
        ) const {
            if ( req.method() == "POST" ) {
                auto logout_claim =
                    req.headers()["__jwt"].subvalue(odin::c_jwt_logout_claim.value());
                if ( logout_claim ) {
                    fostlib::pg::connection cnx{fostgres::connection(config, req)};
                    odin::logout_user(cnx,
                        req.headers()["__odin_reference"].value(),
                        req.headers()["__remote_addr"].value(),
                        req.headers()["__user"].value()
                    );
                    cnx.commit();
                }
                return execute(config, path, req, host);
            } else {
                throw fostlib::exceptions::not_implemented(__func__,
                    "Login requires POST. This should be a 405");
            }
        }
    } c_logout;


}


const fostlib::urlhandler::view &odin::view::logout = c_logout;

