/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/postgres>


namespace odin {


    /// Return the database row for the identity together with the
    /// credentials, but only if the supplied password is correct. If the
    /// credentials failed to match an empty JSON instance (null) is
    /// returned.
    fostlib::json credentials(
        fostlib::pg::connection &cnx,
        const fostlib::string &username,
        const fostlib::string &password);


}

