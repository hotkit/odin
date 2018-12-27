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
        fostlib::insert(config, "then", "odin.test.check_odin_reference_header");
        return config;
    }
    const class check_odin_reference_header : public fostlib::urlhandler::view {
      public:
        check_odin_reference_header() : view("odin.test.check_odin_reference_header") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            FSL_CHECK_EQ(req.headers().exists("__odin_reference"), true);
            return std::make_pair(nullptr, 200);
        }
    } c_check_odin_reference_header;
}


FSL_TEST_FUNCTION(check_odin_ref_header) {
    auto body = std::make_unique<fostlib::binary_body>(
            fostlib::coerce<std::vector<unsigned char>>(
                    fostlib::utf8_string("")));
    fostlib::http::server::request req("POST", "/", std::move(body));
    auto const response = odin::view::middleware_reference(
            configuration(), "/", req, fostlib::host());
}
