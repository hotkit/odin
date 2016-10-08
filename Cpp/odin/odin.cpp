/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/odin.hpp>
#include <odin/nonce.hpp>


const fostlib::module odin::c_odin("odin");


const fostlib::setting<fostlib::string> odin::c_jwt_secret(
    "odin/odin.cpp", "odin", "JWT secret", odin::nonce(), true);

const fostlib::setting<fostlib::string> odin::c_jwt_logout_claim(
    "odin/odin.cpp", "odin", "JWT logout claim", "http://odin.felspar.com/lo", true);

