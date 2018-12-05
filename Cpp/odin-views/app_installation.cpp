/*
    Copyright 2018 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/app.hpp>
#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/views.hpp>

#include <fost/crypto>
#include <fost/insert>
#include <fost/log>
#include <fostgres/sql.hpp>


namespace {

    const fostlib::jcursor apploc{"headers", "__app"};

    const class app_installation : public fostlib::urlhandler::view {
      public:
        app_installation() : view("odin.app.installation") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {

            if (not req[apploc]) {
                throw fostlib::exceptions::not_implemented(
                        __func__,
                        "The odin.app.installation view must be wrapped by an odin.app.secure "
                        "view on the secure path so that there is a valid JWT to find the App ID in");
            }

            /*
                if is_exists(installation_id) and is not registered yet then
                    raise error

           */

            return execute(config["unsecure"], path, req, host);
        }
    } c_app_installation;
}


const fostlib::urlhandler::view &odin::view::app_installation = c_app_installation;
