/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/odin.hpp>
#include <odin/nonce.hpp>

#include <fost/crypto>


fostlib::string odin::nonce() {
    const auto bytes = fostlib::crypto_bytes<24>();
    const auto b64 = fostlib::coerce<fostlib::base64_string>(
        std::vector<unsigned char>(bytes.begin(), bytes.end()));
    return b64.underlying().underlying().c_str();
}

