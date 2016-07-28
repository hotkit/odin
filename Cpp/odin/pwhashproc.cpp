/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/odin.hpp>
#include <odin/pwhashproc.hpp>

#include <fost/crypto>


bool odin::check_password(
    const fostlib::string &password,
    const fostlib::string &hash,
    const fostlib::json &procedure
) {
    const auto salt = fostlib::coerce<std::vector<unsigned char>>(
        fostlib::base64_string(
            fostlib::coerce<fostlib::string>(procedure["salt"]).c_str()));
    const auto hashb = fostlib::coerce<std::vector<unsigned char>>(
        fostlib::base64_string(hash.c_str()));
    const auto hashed = fostlib::pbkdf2_hmac_sha256(
        fostlib::coerce<fostlib::utf8_string>(password),
        salt, 300000, 32);
    return fostlib::crypto_compare(hashed, hashb);
}

