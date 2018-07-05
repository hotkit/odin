/**
    Copyright 2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include <odin/odin.hpp>
#include <odin/pwhashproc.hpp>

#include <fost/crypto>
#include <fost/insert>


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


std::pair<fostlib::string, fostlib::json> odin::hash_password(
    f5::u8view password
) {
    auto salt = fostlib::crypto_bytes<24>();
    fostlib::json process;
    fostlib::insert(process, "name", "pbkdf2-sha256");
    fostlib::insert(process, "rounds", 300000);
    fostlib::insert(process, "length", 32);
    fostlib::insert(process, "salt", fostlib::coerce<fostlib::base64_string>(
        std::vector<unsigned char>(salt.begin(), salt.end())).underlying().underlying().c_str());
    auto hashed = fostlib::string(
        fostlib::coerce<fostlib::base64_string>(
            fostlib::pbkdf2_hmac_sha256(
                password, salt, 300000, 32)).underlying().underlying());
    return std::make_pair(hashed, process);
}

