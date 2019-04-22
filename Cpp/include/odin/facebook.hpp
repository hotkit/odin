/**
    Copyright 2018-2019 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#pragma once

#include <fost/core>
#include <fost/postgres>


namespace odin {


    namespace facebook {


        /// Return facebook app token
        f5::u8string get_app_token(
                const f5::u8string &app_id, const f5::u8string &app_secret);

        /// Check user does authenticated with Facebook
        bool is_user_authenticated(f5::u8view app_token, f5::u8view user_token);

        /// Return user data from Facebook
        fostlib::json get_user_detail(f5::u8view user_token);

        /// Return the database row for the identity together with the
        /// facebook credentials. If the credentials failed to match an empty
        /// JSON instance (null) is returned.
        fostlib::json credentials(
                fostlib::pg::connection &cnx,
                const f5::u8view &facebook_user_id);

        /// Insert Facebook credential of the given user to database, this does
        /// not commit the transaction
        void set_facebook_credentials(
                fostlib::pg::connection &cnx,
                f5::u8view reference,
                f5::u8view identity_id,
                f5::u8view facebook_user_id);


    }


}
