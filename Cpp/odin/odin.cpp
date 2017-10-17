/*
    Copyright 2016-2017 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/

#include <odin/odin.hpp>
#include <odin/nonce.hpp>

#include <fostgres/sql.hpp>


const fostlib::module odin::c_odin("odin");


const fostlib::setting<fostlib::string> odin::c_jwt_secret(
    "odin/odin.cpp", "odin", "JWT secret", odin::nonce(), true);

const fostlib::setting<fostlib::string> odin::c_jwt_logout_claim(
    "odin/odin.cpp", "odin", "JWT logout claim", "http://odin.felspar.com/lo", true);
const fostlib::setting<bool> odin::c_jwt_logout_check(
    "odin/odin.cpp", "odin", "Perform JWT logout check", true, true);


namespace {
    void set_jwt_values_to_cnx_session(fostlib::pg::connection &cnx, const fostlib::http::server::request &req) {
        if ( req.headers().exists("__user") ) {
            const auto &user = req.headers()["__user"];
            cnx.set_session("odin.jwt.sub", user.value());
        }
    }

    struct init {
        init() {
            fostgres::register_cnx_callback(set_jwt_values_to_cnx_session);
        }
    };

    const init i;
}
