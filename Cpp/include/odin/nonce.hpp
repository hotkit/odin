/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>


namespace odin {


    /// Return a string with enough entropy for use in nonces
    /// suitable for salting password hashes or for SHA256-HMAC
    /// secrets. The string contains 32 bytes with 192 bits worth
    /// of entropy.
    fostlib::string nonce();


}

