/**
    Copyright 2019-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <odin/app.hpp>
#include <odin/google.hpp>
#include <odin/thirdparty.hpp>
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

            return odin::thirdparty::login(
                    config, path, req, host, "odin.app.google.login", logger,
                    [](auto &cnx, auto access_token) {
                        return odin::google::get_user_detail(cnx, access_token);
                    },
                    [](auto &cnx, auto const &google_user_id,
                       auto const &app_id) {
                        return odin::google::app_credentials(
                                cnx, google_user_id, app_id);
                    },
                    [](auto &cnx, auto reference, auto identity_id,
                       auto google_user_id) {
                        return odin::google::set_google_credentials(
                                cnx, reference, identity_id, google_user_id);
                    });
        }
    } c_g_login;


}
