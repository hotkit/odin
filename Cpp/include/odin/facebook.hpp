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

        /// Return user data from Facebook
        fostlib::json
                get_user_detail(f5::u8view user_token, fostlib::json config);

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
