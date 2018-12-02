/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/odin.hpp>
#include <odin/views.hpp>

#include <fost/crypto>
#include <fost/test>
#include <fost/exception/parse_error.hpp>


FSL_TEST_SUITE(secure);


namespace {
    fostlib::json configuration() {
        fostlib::json config;
        fostlib::insert(config, "unsecure", "view", "fost.response.401");
        fostlib::insert(
                config, "unsecure", "configuration", "schemes", "Bearer",
                fostlib::json::object_t());
        fostlib::insert(config, "secure", "view", "fost.response.404");
        return config;
    }
}


FSL_TEST_FUNCTION(check_unsecure_01_missing_authz) {
    fostlib::http::server::request req("GET", "/");
    auto response =
            odin::view::secure(configuration(), "/", req, fostlib::host());
    FSL_CHECK_EQ(response.second, 401);
}


FSL_TEST_FUNCTION(check_unsecure_02_wrong_scheme) {
    fostlib::http::server::request req("GET", "/");
    req.headers().set("Authorization", "BASIC uname:aabddsd");
    auto response =
            odin::view::secure(configuration(), "/", req, fostlib::host());
    FSL_CHECK_EQ(response.second, 401);
}


FSL_TEST_FUNCTION(check_unsecure_03_missing_token) {
    fostlib::http::server::request req("GET", "/");
    req.headers().set("Authorization", "Bearer");
    auto response =
            odin::view::secure(configuration(), "/", req, fostlib::host());
    FSL_CHECK_EQ(response.second, 401);
}


FSL_TEST_FUNCTION(check_unsecure_04_wrong_token) {
    fostlib::http::server::request req("GET", "/");
    req.headers().set("Authorization", "Bearer ABACAB");
    auto response =
            odin::view::secure(configuration(), "/", req, fostlib::host());
    FSL_CHECK_EQ(response.second, 401);
}

FSL_TEST_FUNCTION(check_secure) {
    const fostlib::setting<bool> trust_jwt{"odin-views/secure.tests.cpp",
                                           odin::c_jwt_trust, true};
    fostlib::jwt::mint jwt(fostlib::sha256, odin::c_jwt_secret.value());
    jwt.subject("test-user");
    fostlib::http::server::request req("GET", "/");
    req.headers().set("Authorization", ("Bearer " + jwt.token()).c_str());
    auto response =
            odin::view::secure(configuration(), "/", req, fostlib::host());
    FSL_CHECK_EQ(response.second, 404);
}
