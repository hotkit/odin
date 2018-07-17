/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/odin.hpp>
#include <odin/nonce.hpp>

#include <fost/crypto>

#include <chrono>


fostlib::string odin::nonce() {
    const auto base64url = [](auto &&v) {
        fostlib::utf8_string b64u;
        for ( const auto c : v ) {
            if ( c == '+' )
                b64u += '-';
            else if ( c == '/' )
                b64u += '_';
            else if ( c == '=' )
                return b64u;
            else
                b64u += c;
        }
        return b64u;
    };
    const auto bytes = fostlib::crypto_bytes<24>();
    const auto b64 = fostlib::coerce<fostlib::base64_string>(
        std::vector<unsigned char>(bytes.begin(), bytes.end()));
    return base64url(b64).underlying().c_str();
}


fostlib::string odin::reference() {
    const auto time = std::chrono::system_clock::now();
    const auto t_epoch = std::chrono::system_clock::to_time_t(time); // We assume POSIX
    return fostlib::string(std::to_string(t_epoch)) + "-" + nonce();
}


fostlib::pg::connection &odin::reference(fostlib::pg::connection &cnx) {
    return reference(cnx, reference());
}


fostlib::pg::connection &odin::reference(fostlib::pg::connection &cnx, const fostlib::string &ref) {
    /// There can't be an SQL injection attack here because the time
    /// is a number and the nonce is base64 whose alphabet doesn't
    /// include any dangerous characters.
    cnx.exec(fostlib::coerce<fostlib::utf8_string>(
        "SET LOCAL odin.reference='" + ref + "'"));
    return cnx;
}

