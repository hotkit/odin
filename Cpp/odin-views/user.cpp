/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/odin.hpp>
#include <odin/views.hpp>

#include <fost/crypto>
#include <fost/insert>


namespace {


    const class user_unsecure : public fostlib::urlhandler::view {
      public:
        user_unsecure() : view("odin.user.unsecure") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            auto body_data = fostlib::json::parse(req.data()->data());
            if (req.method() == "PUT" || req.method() == "PATCH") {}
            return execute(config, path, req, host);
        }
    } c_unsecure;


}


const fostlib::urlhandler::view &odin::view::user_unsecure = c_unsecure;
