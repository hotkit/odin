/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/odin.hpp>
#include <odin/views.hpp>
#include <odin/credentials.hpp>
#include <fost/crypto>
#include <fost/test>
#include <fost/exception/parse_error.hpp>


FSL_TEST_SUITE(credentials);

namespace {
    fostlib::json configuration() {
        fostlib::json config;
        fostlib::insert(config, "expires", "hours", 72);
        return config;
    }
}

FSL_TEST_FUNCTION(check_return_all_fields_with_new_exp_field) {
    
    fostlib::json payload;
    const fostlib::jcursor iss("iss");
    fostlib::insert(payload, iss, "http://odin.felspar.com/app/app01");

    const fostlib::jcursor sub("sub");
    fostlib::insert(payload, sub, "user01");
  
    fostlib::jwt::mint jwt(fostlib::jwt::alg::HS256, payload);
    fostlib::string secret = odin::c_jwt_secret.value();
    std::string jwt_token = jwt.token(odin::c_jwt_secret.value().data());
    fostlib::string new_jwt_token = odin::renew_jwt(jwt_token, secret, configuration());
    auto load_jwt = fostlib::jwt::token::load(secret, new_jwt_token);
    
    FSL_CHECK_EQ(load_jwt.value().payload[iss], payload[iss]);
    FSL_CHECK_EQ(load_jwt.value().payload[sub], payload[sub]);

    const fostlib::jcursor exp("exp");
    FSL_CHECK(load_jwt.value().payload.has_key(exp));
}

FSL_TEST_FUNCTION(check_can_renew_even_have_old_exp_field) {
    
    fostlib::json payload;
    const fostlib::jcursor iss("iss");
    fostlib::insert(payload, iss, "http://odin.felspar.com/app/app01");

    const fostlib::jcursor sub("sub");
    fostlib::insert(payload, sub, "user01");
  
    const fostlib::jcursor exp("exp");
    fostlib::insert(payload, exp, 1561819147);

    fostlib::jwt::mint jwt(fostlib::jwt::alg::HS256, payload);
    fostlib::string secret = odin::c_jwt_secret.value();
    std::string jwt_token = jwt.token(odin::c_jwt_secret.value().data());
    fostlib::string new_jwt_token = odin::renew_jwt(jwt_token, secret, configuration());
    auto load_jwt = fostlib::jwt::token::load(secret, new_jwt_token);
    
    FSL_CHECK_NEQ(load_jwt.value().payload[exp], payload[exp]);

}
