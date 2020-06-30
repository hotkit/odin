/**
    Copyright 2018-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once

#include <fost/core>
#include <fost/log>
#include <fost/http>
#include <fost/postgres>
#include <fost/urlhandler>


namespace odin {


    namespace thirdparty {


        /// Return database user id that match with given email
        /// if that user has thirdparty credential (apple/facebook/google)
        /// If failed to match given email or multiple match return {}
        std::optional<f5::u8string> email_owner_identity_id(
                fostlib::pg::connection &cnx, fostlib::string email);

        /// Because each third party login have same mechanism of merge
        /// set credential, name and app_uer then we create generic function
        /// receive lamda function that different between each thirdparty 
        std::pair<boost::shared_ptr<fostlib::mime>, int> login(
            const fostlib::json &config,
            const fostlib::string &path,
            fostlib::http::server::request &req,
            const fostlib::host &host,
            const fostlib::string &view_name,
            fostlib::log::detail::log_object &logger,
            const std::function<
                    fostlib::json(fostlib::pg::connection &, f5::u8view)>
                    &thirdparty_user_detail,
            const std::function<fostlib::json(
                    fostlib::pg::connection &, const f5::u8view &, const f5::u8view &)>
                    &thirdparty_app_credential,
            const std::function<void(
                    fostlib::pg::connection &, f5::u8view, f5::u8view, f5::u8view)>
                    &set_thirdparty_credentials);


    }


}
