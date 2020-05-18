/**
    Copyright 2016-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <fost/postgres>


namespace odin {


    /// Return a string with enough entropy for use in nonces
    /// suitable for salting password hashes or for SHA256-HMAC
    /// secrets. The string contains 32 bytes with 192 bits worth
    /// of entropy.
    fostlib::string nonce();


    /// Return a reference value to be used with SQL commands
    fostlib::string reference();


    /// Sets a new reference on the database connection so that
    /// updates can be tied together.
    fostlib::pg::connection &reference(fostlib::pg::connection &);


    /// Sets the specified reference on a connection.
    fostlib::pg::connection &
            reference(fostlib::pg::connection &, const fostlib::string &);


}
