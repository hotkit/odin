/*
    Copyright 2018 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/views.hpp>
#include <fost/test>
#include <fost/exception/parse_error.hpp>


FSL_TEST_SUITE(password_hash);

namespace {
    fostlib::json configuration() {
        fostlib::json config;
        fostlib::insert(config, "hash", "password");
        fostlib::insert(config, "verify", "password2");
        fostlib::insert(config, "then", "fost.response.200");
        return config;
    }
}

FSL_TEST_FUNCTION(check_config) {
    fostlib::http::server::request req("POST", "/");
    FSL_CHECK_EXCEPTION(
            odin::view::password_hash(
                    fostlib::json(), "/", req, fostlib::host()),
            fostlib::exceptions::not_implemented &);
}


FSL_TEST_FUNCTION(check_verify_key) {
    auto body = std::make_unique<fostlib::binary_body>(
            fostlib::coerce<std::vector<unsigned char>>(fostlib::utf8_string(
                    "{\"password\": \"PWD\", \"password2\": \"PWD2\"}")));
    fostlib::http::server::request req("POST", "/", std::move(body));
    auto const response = odin::view::password_hash(
            configuration(), "/", req, fostlib::host());
    FSL_CHECK_EQ(response.second, 422);
}

FSL_TEST_FUNCTION(check_hash_success) {
    auto body = std::make_unique<fostlib::binary_body>(
            fostlib::coerce<std::vector<unsigned char>>(
                    fostlib::utf8_string("{\"password\": \"password\", "
                                         "\"password2\": \"password\"}")));
    fostlib::http::server::request req("POST", "/", std::move(body));
    auto const response = odin::view::password_hash(
            configuration(), "/", req, fostlib::host());
    FSL_CHECK_EQ(response.second, 200);
}
