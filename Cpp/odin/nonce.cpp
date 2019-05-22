/**
    Copyright 2016-2019, Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include <odin/odin.hpp>
#include <odin/nonce.hpp>

#include <fost/crypto>

#include <chrono>


fostlib::string odin::nonce() {
    return fostlib::nonce24b64u().underlying().underlying();
}


fostlib::string odin::reference() {
    return fostlib::timestamp_nonce24b64u().underlying().underlying();
}


fostlib::pg::connection &odin::reference(fostlib::pg::connection &cnx) {
    return reference(cnx, reference());
}


fostlib::pg::connection &odin::reference(
        fostlib::pg::connection &cnx, const fostlib::string &ref) {
    /// There can't be an SQL injection attack here because the time
    /// is a number and the nonce is base64 whose alphabet doesn't
    /// include any dangerous characters.
    cnx.exec(fostlib::coerce<fostlib::utf8_string>(
            "SET LOCAL odin.reference='" + ref + "'"));
    return cnx;
}
