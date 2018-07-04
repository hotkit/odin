/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/crypto>
#include <fost/internet>
#include <fost/postgres>


namespace odin {
    
    /// Create the given user
    fostlib::json create_user(
        fostlib::pg::connection &cnx,
        const fostlib::string &username,
        const fostlib::string &password
    );
}
