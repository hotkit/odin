/**
    Copyright 2018 Felspar Co Ltd. <http://odin.felspar.com/>

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
    f5::u8view username
) {
    fg::json user_values;
    fostlib::insert(user_values, "reference", reference);
    fostlib::insert(user_values, "identity_id", username);
    cnx.insert("odin.identity_ledger", user_values);
}


void odin::set_password(
    fostlib::pg::connection &cnx,
    f5::u8view reference,
    f5::u8view username,
    f5::u8view password
) {
    auto hashed = odin::hash_password(password);
    save_hash(cnx, reference, username, hashed.first, hashed.second);
}


void odin::save_hash(
    fostlib::pg::connection &cnx,
    f5::u8view reference,
    f5::u8view username,
    f5::u8view hash,
    fostlib::json process
) {
    fg::json user_values;
    fostlib::insert(user_values, "reference", reference);
    fostlib::insert(user_values, "identity_id", username);
    fostlib::insert(user_values, "password", hash);
    fostlib::insert(user_values, "process", process);
    cnx.insert("odin.credentials_password_ledger", user_values);
}


bool odin::does_user_exist(fostlib::pg::connection &cnx, f5::u8view username) {
    static const fostlib::string sql("SELECT * FROM odin.identity WHERE id=$1");
    auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{username});
    auto &rs = data.second;
    return rs.begin() != rs.end();
}


void odin::set_full_name(
    fostlib::pg::connection &cnx,
    f5::u8view reference,
    f5::u8view username,
    f5::u8view full_name
) {
    fg::json user_values;
    fostlib::insert(user_values, "reference", reference);
    fostlib::insert(user_values, "identity_id", username);
    fostlib::insert(user_values, "full_name", full_name);
    cnx.insert("odin.identity_full_name_ledger", user_values);
}


void odin::set_email(
    fostlib::pg::connection &cnx,
    f5::u8view reference,
    f5::u8view username,
    f5::u8view email
) {
    fg::json user_values;
    fostlib::insert(user_values, "reference", reference);
    fostlib::insert(user_values, "identity_id", username);
    fostlib::insert(user_values, "email", email);
    cnx.insert("odin.identity_email_ledger", user_values);
}
