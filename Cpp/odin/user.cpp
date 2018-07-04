/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/

#include <odin/fg/native.hpp>
#include <odin/nonce.hpp>
#include <odin/user.hpp>
#include <odin/pwhashproc.hpp>

#include <fost/insert>


fostlib::json odin::create_user(
    fostlib::pg::connection &cnx,
    const fostlib::string &username
) {
    fg::json user_values;
    // odin::reference(cnx) does not work
    // fostlib::insert(user_values, "reference", odin::reference(cnx));
    fostlib::insert(user_values, "reference", odin::reference());
    fostlib::insert(user_values, "identity_id", username);
    cnx.insert("odin.identity_ledger", user_values);
    cnx.commit();
    return fostlib::json();
}

fostlib::json odin::create_user(
    fostlib::pg::connection &cnx,
    const fostlib::string &username,
    const fostlib::string &password
) {
    fg::json user_values;
    // odin::reference(cnx) does not work
    // fostlib::insert(user_values, "reference", odin::reference(cnx));
    fostlib::insert(user_values, "reference", odin::reference());
    fostlib::insert(user_values, "identity_id", username);
    cnx.insert("odin.identity_ledger", user_values);

    auto hashed = odin::set_password(password);
    fostlib::insert(user_values, "password", hashed.first);
    fostlib::insert(user_values, "process", hashed.second);
    cnx.insert("odin.credentials_password_ledger", user_values);
    cnx.commit();
    return fostlib::json();
}
