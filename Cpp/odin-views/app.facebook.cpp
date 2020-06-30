/**
    Copyright 2019-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */

#include <odin/app.hpp>
#include <odin/facebook.hpp>
#include <odin/thirdparty.hpp>
#include <odin/views.hpp>

#include <fost/log>


namespace {


    fostlib::module const c_odin_app_facebook{odin::c_odin_app, "facebook"};


    const class fb_login : public fostlib::urlhandler::view {
      public:
        fb_login() : view("odin.app.facebook.login") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {

            auto logger{fostlib::log::debug(c_odin_app_facebook)};
            return odin::thirdparty::login(
                    config, path, req, host, "odin.app.facebook.login", logger,
                    [](auto &cnx, auto access_token) {
                        return odin::facebook::get_user_detail(
                                cnx, access_token);
                    },
                    [](auto &cnx, auto const &facebook_user_id,
                       auto const &app_id) {
                        return odin::facebook::app_credentials(
                                cnx, facebook_user_id, app_id);
                    },
                    [](auto &cnx, auto reference, auto identity_id,
                       auto facebook_user_id) {
                        return odin::facebook::set_facebook_credentials(
                                cnx, reference, identity_id, facebook_user_id);
                    });
        }
    } c_fb_login;


}
