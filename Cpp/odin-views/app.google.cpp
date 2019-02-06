/**
    Copyright 2019 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include <odin/app.hpp>
#include <odin/views.hpp>

#include <fost/log>


namespace {


    fostlib::module const c_odin_app_google{odin::c_odin_app, "google"};


    const class g_login : public fostlib::urlhandler::view {
      public:
        g_login() : view("odin.app.google.login") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            auto logger{fostlib::log::debug(c_odin_app_google)};
            if (req.method() != "POST") {
                fostlib::json config;
                fostlib::insert(config, "view", "fost.response.405");
                fostlib::push_back(config, "configuration", "allow", "POST");
                return execute(config, path, req, host);
            }
            if (not req.headers().exists("__app")
                || not req.headers().exists("__user")) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "The odin.app.google.login view must be wrapped by an "
                        "odin.app.secure view on the secure path so that there "
                        "is a valid JWT to find the App ID in");
            }
            logger("__app", req.headers()["__app"]);
            logger("__user", req.headers()["__user"]);
            throw fostlib::exceptions::not_implemented(__PRETTY_FUNCTION__);
        }

    } c_g_login;


}
