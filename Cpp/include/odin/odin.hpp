/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>


namespace odin {


    /// Module
    extern const fostlib::module c_odin;


    /// The secret used for JWT tokens
    extern const fostlib::setting<fostlib::string> c_jwt_secret;

    /// The JWT claim for the log out count
    extern const fostlib::setting<fostlib::string> c_jwt_logout_claim;


}

