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
        fostlib::insert(config, "then", "odin.test.check_request_header");
        return config;
    }
    const class check_request_header : public fostlib::urlhandler::view {
      public:
        check_request_header() : view("odin.test.check_request_header") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            FSL_CHECK_EQ(req.headers().exists("__hash"), true);
            const auto hash = req.headers()["__hash"].value();

            FSL_CHECK_EQ(req.headers().exists("__hash_process"), true);
            const auto hash_process = fostlib::json::parse(
                    req.headers()["__hash_process"].value());
            FSL_CHECK_EQ(hash_process["length"], fostlib::json{32});
            FSL_CHECK_EQ(hash_process["name"], fostlib::json{"pbkdf2-sha256"});
            FSL_CHECK_EQ(hash_process["rounds"], fostlib::json{300000});

            fostlib::mime::mime_headers headers;
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    fostlib::json::unparse(fostlib::json{}, true), headers,
                    "application/json"));
            return std::make_pair(response, 200);
        }
    } c_check_request_header;
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
