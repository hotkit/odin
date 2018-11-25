/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/urlhandler>


namespace odin {


    namespace view {


        extern const fostlib::urlhandler::view &login, &logout, &secure,
                &user_unsecure, &registration, &facebook, &facebook_link,
                &google, &app_login, &app_secure;


    }


}
