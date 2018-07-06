/**
    Copyright 2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#pragma once


#include <fost/postgres>


namespace odin {
    

    /// Create the given user, this does not commit the new user
    void create_user(fostlib::pg::connection &cnx, f5::u8view reference, f5::u8view username);

    /// Hash the current password with the default procedures,
    /// Save password to the Database for the given user.
    /// This does not commit the new password
    void set_password(fostlib::pg::connection &cnx, f5::u8view reference, f5::u8view username, f5::u8view password);

    /// Save a password hash and process to the database. Does not commit
    /// the transaction.
    void save_hash(fostlib::pg::connection &cnx, f5::u8view reference,
        f5::u8view username, f5::u8view hash, fostlib::json process);

    /// Check user does already exists in the database
    bool does_user_exist(fostlib::pg::connection &cnx, f5::u8view username);
}
