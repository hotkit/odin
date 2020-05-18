/**
    Copyright 2016-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <fost/core>


namespace odin {


    /// Return true if the password matches the specified procedure
    bool check_password(
            const fostlib::string &password,
            const fostlib::string &hash,
            const fostlib::json &procedure);


    /// The number of hashing rounds to be used for setting new passwords
    extern fostlib::setting<int64_t> const c_hash_rounds;

    /// Set the given user's password
    std::pair<fostlib::string, fostlib::json> hash_password(f5::u8view password);


}
