/*
    Copyright 2016-2017 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>
#include <fost/postgres>

namespace odin {


    /// Module
    extern const fostlib::module c_odin;


    /// See odin.cpp for default values.

    /// The secret used for JWT tokens. Always pick a new value for this as
    /// the default will change each time the mengmom is restarted.
    extern const fostlib::setting<fostlib::string> c_jwt_secret;

    /// Set this to true to skip database check against user JWT.
    extern const fostlib::setting<bool> c_jwt_trust;

    /// The JWT claim for the log out count.
    extern const fostlib::setting<fostlib::string> c_jwt_logout_claim;
    /// Turn off the logout count check. Saves a database connection and an
    /// SQL `SELECT`, but means that revokation of JWTs won't be noticed.
    extern const fostlib::setting<bool> c_jwt_logout_check;

    /// The JWT claim for embedded permissions
    extern const fostlib::setting<fostlib::string> c_jwt_permissions_claim;

    /// The secret used for JWT forgotten password reset tokens. Always pick a
    /// new value for this as the default will change each time the mengmom is
    /// restarted.
    extern const fostlib::setting<fostlib::string>
            c_jwt_reset_forgotten_password_secret;

    /// Facebook AppID
    extern const fostlib::setting<fostlib::string> c_facebook_app_id;

    /// Facebook Secret
    extern const fostlib::setting<fostlib::string> c_facebook_secret;

    /// Facebook API Endpoint
    extern const fostlib::setting<fostlib::string> c_facebook_endpoint;

    /// Google AUD
    extern const fostlib::setting<fostlib::json> c_google_aud;

    /// Application namespace
    extern const fostlib::setting<fostlib::string> c_app_namespace;

    /// Check module is enabled in the database, does not commit the transaction
    bool is_module_enabled(fostlib::pg::connection &cnx, f5::u8view module_name);
}
