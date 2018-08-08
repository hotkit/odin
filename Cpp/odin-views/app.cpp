/**
    Copyright 2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/

#include <odin/credentials.hpp>
#include <odin/odin.hpp>
#include <odin/nonce.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <fost/exception/parse_error.hpp>
#include <fost/insert>
#include <fost/log>
#include <fost/mailbox>
#include <fostgres/sql.hpp>


namespace {

    const fostlib::module c_odin_app(odin::c_odin, "app.cpp");

    const class app_exchange : public fostlib::urlhandler::view {
    public:
        app_exchange()
        : view("odin.app.exchange") {
        }

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator () (
            const fostlib::json &config,
            const fostlib::string &path,
            fostlib::http::server::request &req,
            const fostlib::host &host
        ) const {
            if ( req.method() != "POST" )
                throw fostlib::exceptions::not_implemented(__func__, "App exchange require POST");
            
            fostlib::mime::mime_headers headers;
            boost::shared_ptr<fostlib::mime> response(
                    new fostlib::text_body(fostlib::utf8_string("Exchange"),
                        headers, L"text/html"));
            return std::make_pair(response, 200);
        }
    } c_app_exchange;


}


const fostlib::urlhandler::view &odin::view::app_exchange = c_app_exchange;

