/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/urlhandler>


namespace {


    const class login : public fostlib::urlhandler::view {
        public:
            login()
            : view("odin.login") {
            }

            std::pair<boost::shared_ptr<fostlib::mime>, int> operator () (
                const fostlib::json &config, const fostlib::string &,
                fostlib::http::server::request &req,
                const fostlib::host &
            ) const {
                throw fostlib::exceptions::not_implemented(__FUNCTION__);
            }
    } c_login;


}


