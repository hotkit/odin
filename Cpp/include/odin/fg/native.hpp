/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fostgres/fg/fg.extension.hpp>
#include <fost/postgres>


namespace odin {


    const extern fostlib::module c_odin_fg;


    /// Connect and set hte reference option on the connection
    fostlib::pg::connection connect(fg::frame &);


    namespace lib {
        const extern fg::frame::builtin assign, expire, group, jwt,
            jwt_payload, membership, permission, superuser, user;
    }


}

