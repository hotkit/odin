/**
    Copyright 2018-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */

#include <fostgres/sql.hpp>

#include <odin/fg/native.hpp>
#include <odin/nonce.hpp>
#include <odin/user.hpp>
#include <odin/pwhashproc.hpp>

#include <fost/insert>


void odin::create_user(
        fostlib::pg::connection &cnx,
        f5::u8view reference,
        f5::u8view identity_id) {
    fg::json user_values;
    fostlib::insert(user_values, "reference", reference);
    fostlib::insert(user_values, "identity_id", identity_id);
    cnx.insert("odin.identity_ledger", user_values);
}

void odin::create_user(fostlib::pg::connection &cnx, f5::u8view identity_id) {
    odin::create_user(cnx, identity_id, identity_id);
}

void odin::logout_user(
        fostlib::pg::connection &cnx,
        f5::u8view reference,
        f5::u8view source_address,
        f5::u8view identity_id) {
    fostlib::json row;
    fostlib::insert(row, "identity_id", identity_id);
    fostlib::insert(row, "reference", reference);
    fostlib::insert(row, "source_address", source_address);
    cnx.insert("odin.logout_ledger", row);
}


void odin::set_password(
        fostlib::pg::connection &cnx,
        f5::u8view reference,
        f5::u8view identity_id,
        f5::u8view username,
        f5::u8view password) {
    auto hashed = odin::hash_password(password);
    save_credential(
            cnx, reference, identity_id, username, hashed.first, hashed.second);
}


void odin::save_credential(
        fostlib::pg::connection &cnx,
        f5::u8view reference,
        f5::u8view identity_id,
        f5::u8view username,
        f5::u8view hash,
        fostlib::json process) {
    fg::json user_values;
    fostlib::insert(user_values, "reference", reference);
    fostlib::insert(user_values, "identity_id", identity_id);
    fostlib::insert(user_values, "login", username);
    fostlib::insert(user_values, "password", hash);
    fostlib::insert(user_values, "process", process);
    cnx.insert("odin.credentials_password_ledger", user_values);
}


bool odin::does_user_exist(fostlib::pg::connection &cnx, f5::u8view username) {
    static const fostlib::string sql(
            "SELECT * FROM odin.credentials WHERE login=$1");
    auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{username});
    auto &rs = data.second;
    return rs.begin() != rs.end();
}


void odin::set_full_name(
        fostlib::pg::connection &cnx,
        f5::u8view reference,
        f5::u8view identity_id,
        f5::u8view full_name) {
    fg::json user_values;
    fostlib::insert(user_values, "reference", reference);
    fostlib::insert(user_values, "identity_id", identity_id);
    fostlib::insert(user_values, "full_name", full_name);
    cnx.insert("odin.identity_full_name_ledger", user_values);
}


void odin::set_email(
        fostlib::pg::connection &cnx,
        f5::u8view reference,
        f5::u8view identity_id,
        fostlib::email_address email) {
    fg::json user_values;
    fostlib::insert(user_values, "reference", reference);
    fostlib::insert(user_values, "identity_id", identity_id);
    fostlib::insert(user_values, "email", email.email());
    cnx.insert("odin.identity_email_ledger", user_values);
}


void odin::set_installation_id(
        fostlib::pg::connection &cnx,
        f5::u8view reference,
        f5::u8view identity_id,
        f5::u8view installation_id) {
    fg::json user_values;
    fostlib::insert(user_values, "reference", reference);
    fostlib::insert(user_values, "identity_id", identity_id);
    fostlib::insert(user_values, "installation_id", installation_id);
    cnx.insert("odin.identity_installation_id_ledger", user_values);
}


bool odin::does_email_exist(fostlib::pg::connection &cnx, fostlib::string email) {
    static const fostlib::string sql(
            "SELECT email FROM odin.identity WHERE email=$1");
    auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{email});
    auto &rs = data.second;
    return rs.begin() != rs.end();
}
