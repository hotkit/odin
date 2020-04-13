/**
    Copyright 2016-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <odin/odin.hpp>
#include <odin/views.hpp>
#include <odin/credentials.hpp>
#include <fost/test>

FSL_TEST_SUITE(credentials);

namespace {
    fostlib::json configuration() {
        fostlib::json config;
        fostlib::insert(config, "expires", "hours", 72);
        return config;
    }

    fostlib::json configuration_with_app_id() {
        fostlib::json config;
        fostlib::insert(config, "expires", "hours", 72);
        fostlib::insert(config, "app_id", "app02");
        return config;
    }

    const fostlib::timestamp c_epoch(1970, 1, 1);
}

FSL_TEST_FUNCTION(check_can_renew_jwt_with_non_app_jwt) {

    fostlib::json payload;
    fostlib::insert(payload, "sub", "user01");
    fostlib::insert(payload, "fullname", "test01");
    const auto old_exp =
            fostlib::timestamp::now() + boost::posix_time::hours(24);
    fostlib::insert(payload, "exp", (old_exp - c_epoch).total_seconds());
    fostlib::jwt::mint jwt(fostlib::jwt::alg::HS256, payload);

    fostlib::http::server::request req("GET", "/");
    req.headers().set(
            "Authorization",
            ("Bearer " + jwt.token(odin::c_jwt_secret.value().data())).c_str());
    req.headers().set("__user", "user01");

    auto const [response, http_status_code] =
            odin::view::jwt_renewal(configuration(), "/", req, fostlib::host());
    auto jwt_token = response->body_as_string();
    auto load_jwt =
            fostlib::jwt::token::load(odin::c_jwt_secret.value(), jwt_token);

    FSL_CHECK(load_jwt.has_value());
    FSL_CHECK_EQ(load_jwt->payload["sub"], payload["sub"]);
    FSL_CHECK_EQ(load_jwt->payload["fullname"], payload["fullname"]);
    FSL_CHECK_NEQ(load_jwt->payload["exp"], payload["exp"]);
    FSL_CHECK_EQ(http_status_code, 200);
}


FSL_TEST_FUNCTION(
        check_return_app_jwt_when_renew_non_app_jwt_with_app_id_is_set) {

    fostlib::json payload;
    fostlib::insert(payload, "sub", "user01");
    fostlib::insert(payload, "fullname", "test01");
    const auto old_exp =
            fostlib::timestamp::now() + boost::posix_time::hours(24);
    fostlib::insert(payload, "exp", (old_exp - c_epoch).total_seconds());
    fostlib::jwt::mint jwt(fostlib::jwt::alg::HS256, payload);

    fostlib::http::server::request req("GET", "/");
    req.headers().set(
            "Authorization",
            ("Bearer " + jwt.token(odin::c_jwt_secret.value().data())).c_str());
    req.headers().set("__user", "user01");

    auto const [response, http_status_code] = odin::view::jwt_renewal(
            configuration_with_app_id(), "/", req, fostlib::host());
    auto jwt_token = response->body_as_string();
    auto app_id = fostlib::coerce<fostlib::string>(
            configuration_with_app_id()["app_id"]);
    auto load_jwt = fostlib::jwt::token::load(
            odin::c_jwt_secret.value() + app_id, jwt_token);

    FSL_CHECK(load_jwt.has_value());
    FSL_CHECK_EQ(
            load_jwt->payload["iss"], "http://odin.felspar.com/app/" + app_id);
    FSL_CHECK_EQ(load_jwt->payload["sub"], payload["sub"]);
    FSL_CHECK_EQ(load_jwt->payload["fullname"], payload["fullname"]);
    FSL_CHECK_NEQ(load_jwt->payload["exp"], payload["exp"]);
    FSL_CHECK_EQ(http_status_code, 200);
}

FSL_TEST_FUNCTION(check_can_renew_jwt_with_app_jwt) {
    fostlib::string app_id = "app01";
    fostlib::json payload;
    fostlib::insert(payload, "iss", "http://odin.felspar.com/app/app01");
    fostlib::insert(payload, "sub", "user01");
    fostlib::insert(payload, "app", app_id);
    const auto old_exp =
            fostlib::timestamp::now() + boost::posix_time::hours(24);
    fostlib::insert(payload, "exp", (old_exp - c_epoch).total_seconds());
    fostlib::jwt::mint jwt(fostlib::jwt::alg::HS256, payload);
    fostlib::http::server::request req("GET", "/");
    auto secret = odin::c_jwt_secret.value() + app_id;
    req.headers().set(
            "Authorization", ("Bearer " + jwt.token(secret.data())).c_str());
    req.headers().set("__user", "user01");
    req.headers().set("__app", app_id);

    auto const [response, http_status_code] =
            odin::view::jwt_renewal(configuration(), "/", req, fostlib::host());
    auto jwt_token = response->body_as_string();
    auto load_jwt = fostlib::jwt::token::load(secret, jwt_token);

    FSL_CHECK(load_jwt.has_value());
    FSL_CHECK_EQ(load_jwt->payload["iss"], payload["iss"]);
    FSL_CHECK_EQ(load_jwt->payload["sub"], payload["sub"]);
    FSL_CHECK_EQ(load_jwt->payload["app"], payload["app"]);
    FSL_CHECK_EQ(load_jwt->payload["exp"], load_jwt->payload["exp"]);
    FSL_CHECK_EQ(http_status_code, 200);
}
