/**
    Copyright 2018-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once

#include <fost/crypto>
#include <fost/postgres>


namespace odin {


    extern fostlib::module const c_odin_app;


    namespace app {


        /// Return the database row for the app
        fostlib::json get_detail(
                fostlib::pg::connection &cnx, const f5::u8view app_id);

        /// Mint App specific JWT for this user and set common fields on it
        std::pair<fostlib::utf8_string, fostlib::timestamp> mint_user_jwt(
                const f5::u8view identity_id,
                const f5::u8view app_id,
                const fostlib::timediff,
                fostlib::json payload = fostlib::json{});

        /// Save the given app user, this does not commit the transaction
        void save_app_user(
                fostlib::pg::connection &cnx,
                f5::u8view reference,
                const f5::u8view app_id,
                const f5::u8view identity_id,
                const f5::u8view app_user_id);

        /// Return app user detail
        fostlib::json get_app_user(
                fostlib::pg::connection &cnx,
                const f5::u8view app_id,
                const f5::u8view identity_id);

        /// Save installation ID if the given user to database, this does not
        /// commit the transaction
        void set_installation_id(
                fostlib::pg::connection &cnx,
                f5::u8view reference,
                f5::u8view app_id,
                f5::u8view identity_id,
                f5::u8view installation_id);

        /// Save app_user_id if the given user to database, this does not
        /// commit the transaction
        void set_app_user_id(
                fostlib::pg::connection &cnx,
                f5::u8view reference,
                f5::u8view app_id,
                f5::u8view identity_id,
                f5::u8view app_user_id);


    }


}
