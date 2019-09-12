/**
    Copyright 2016-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <fostgres/fg/fg.extension.hpp>
#include <fost/postgres>


namespace odin {


    const extern fostlib::module c_odin_fg;


    /// Connect and set hte reference option on the connection
    fostlib::pg::connection connect(fg::frame &);


    namespace lib {
        const extern fg::frame::builtin assign, expire, group, hash,
                mint_login_jwt, mint_jwt, mint_reset_password_jwt, jwt_payload,
                membership, permission, superuser, user;
    }


}
