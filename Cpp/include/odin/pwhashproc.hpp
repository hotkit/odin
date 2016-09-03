/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>


namespace odin {


    /// Return true if the password matches the specified procedure
    bool check_password(
        const fostlib::string &password,
        const fostlib::string &hash,
        const fostlib::json &procedure);


    /// Set the given user's password
    std::pair<fostlib::string, fostlib::json> set_password(
        const fostlib::string &password);


}

