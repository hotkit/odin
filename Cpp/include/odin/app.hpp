/**
    Copyright 2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#pragma once

#include <fost/crypto>
#include <fost/postgres>


namespace odin {

    namespace app {

        /// Return the database row for the app
        fostlib::json get_detail(fostlib::pg::connection &cnx, const f5::u8view app_id);

        /// Mint App specific JWT for this user and set common fields on it
        fostlib::jwt::mint mint_user_jwt(const fostlib::json &user, const fostlib::json &app,
            fostlib::json payload = fostlib::json{});

        /// Save the given app user, this does not commit the transaction
        void save_app_user(fostlib::pg::connection &cnx, f5::u8view reference,
            const f5::u8view identity_id, const f5::u8view app_id);

    }


}
