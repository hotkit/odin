/**
    Copyright 2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#pragma once


#include <fost/mailbox>
#include <fost/postgres>


namespace odin {


    /// Create the given user, this does not commit the new user
    void create_user(fostlib::pg::connection &cnx, f5::u8view reference, f5::u8view username);

    /// Logout the given user, this does not commit the transaction
    void logout_user(fostlib::pg::connection &cnx, f5::u8view reference,
        f5::u8view source_address, f5::u8view identity_id);

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

    /// Save full name of the given user to database, this does not commit the transaction
    void set_full_name(fostlib::pg::connection &cnx, f5::u8view reference, f5::u8view username,
        f5::u8view full_name);

    /// Save email of the given user to database, this does not commit the transaction
    void set_email(fostlib::pg::connection &cnx, f5::u8view reference, f5::u8view username,
        fostlib::email_address email);

    /// Save installation ID if the given user to datavase, this does not commit the transaction
    void set_installation_id(fostlib::pg::connection &cnx, f5::u8view reference, f5::u8view identity_id,
        f5::u8view installation_id);

    /// Check email already exists in the database
    bool does_email_exist(fostlib::pg::connection &cnx, fostlib::string email);

}
