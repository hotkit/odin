/**
    Copyright 2018-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once

#include <fost/core>
#include <fost/postgres>

namespace odin {


    namespace google {


        /// Return user data from Google
        fostlib::json get_user_detail(fostlib::pg::connection &cnx, f5::u8view user_token);

        /// Return the database row for the identity together with the
        /// google credentials. If the credentials failed to match an empty
        /// JSON instance (null) is returned.
        fostlib::json credentials(
                fostlib::pg::connection &cnx, const f5::u8view &google_user_id);

        /// Return the database row for the identity together with the
        /// google credentials and app_user. If the credentials failed to match
        /// an empty JSON instance (null) is returned.
        fostlib::json app_credentials(
                fostlib::pg::connection &cnx,
                const f5::u8view &google_user_id,
                const f5::u8view &app_id);

        /// Save google credential of the given user to database, this does not
        /// commit the transaction
        void set_google_credentials(
                fostlib::pg::connection &cnx,
                f5::u8view reference,
                f5::u8view identity_id,
                f5::u8view google_user_id);

        /// Return database user id that match with given email
        /// if that user has google credential
        /// If failed to match given email or multiple match return {}
        std::optional<f5::u8string> email_owner_identity_id(
                fostlib::pg::connection &cnx, fostlib::string email);


    }


}
