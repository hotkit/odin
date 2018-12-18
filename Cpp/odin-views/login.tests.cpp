/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/views.hpp>
#include <fost/test>
#include <fost/exception/parse_error.hpp>


FSL_TEST_SUITE(login);


FSL_TEST_FUNCTION(check_body) {
    fostlib::http::server::request req("POST", "/");
    FSL_CHECK_EXCEPTION(
            odin::view::login(fostlib::json(), "/", req, fostlib::host()),
            fostlib::exceptions::parse_error &);
}


FSL_TEST_FUNCTION(chacks_args) {
    auto body = std::make_unique<fostlib::binary_body>(
            fostlib::coerce<std::vector<unsigned char>>(
                    fostlib::utf8_string("{}")));
    fostlib::http::server::request req("POST", "/", std::move(body));
    FSL_CHECK_EXCEPTION(
            odin::view::login(fostlib::json(), "/", req, fostlib::host()),
            fostlib::exceptions::not_implemented &);
}
