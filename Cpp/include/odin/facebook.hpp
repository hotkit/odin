/**
    Copyright 2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#pragma once

#include <fost/core>
#include <fost/postgres>

namespace odin {


    namespace facebook {


        /// Return facebook app token
        fostlib::string get_app_token();

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

        /// Save facebook credential of the given user to database, this does
        /// not commit the transaction
        void set_facebook_credentials(
                fostlib::pg::connection &cnx,
                f5::u8view reference,
                f5::u8view identity_id,
                f5::u8view facebook_user_id);

    }


}
