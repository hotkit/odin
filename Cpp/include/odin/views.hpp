/**
    Copyright 2016-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <fost/urlhandler>


namespace odin {


    namespace view {


        extern const fostlib::urlhandler::view &login, &logout, &secure,
                &user_unsecure, &registration, &facebook, &facebook_link,
                &google, &app_installation, &app_login, &app_mint, &app_secure,
                &password_hash, &middleware_reference, &jwt_renewal, &app_user;
    }
}
