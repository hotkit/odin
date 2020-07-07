/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

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


    }


}
