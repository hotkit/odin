/**
    Copyright 2016-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/views.hpp>

#include <fost/crypto>
#include <fost/insert>
#include <fost/log>
#include <fostgres/sql.hpp>


namespace {


    const class middleware_reference : public fostlib::urlhandler::view {
      public:
        middleware_reference() : view("odin.middleware.reference") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            req.headers().set("__odin_reference", odin::reference());
            return execute(config["then"], path, req, host);
        }
    } c_middleware_reference;


}


const fostlib::urlhandler::view &odin::view::middleware_reference =
        c_middleware_reference;
