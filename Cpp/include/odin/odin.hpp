/*
    Copyright 2016-2017 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>


namespace odin {


    /// Module
    extern const fostlib::module c_odin;


    /// See odin.cpp for default values.

    /// The secret used for JWT tokens. Always pick a new value for this as
    /// the default will change each time the mengmom is restarted.
    extern const fostlib::setting<fostlib::string> c_jwt_secret;

    /// The JWT claim for the log out count.
    extern const fostlib::setting<fostlib::string> c_jwt_logout_claim;
    /// Turn off the logout count check. Saves a database connection and an
    /// SQL `SELECT`, but means that revokation of JWTs won't be noticed.
    extern const fostlib::setting<bool> c_jwt_logout_check;


}

