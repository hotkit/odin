/**
    Copyright 2016-2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include <odin/fg/native.hpp>
#include <odin/nonce.hpp>
#include <odin/user.hpp>
#include <odin/pwhashproc.hpp>

#include <fost/datetime>
#include <fost/insert>
#include <fost/log>


const fg::frame::builtin odin::lib::superuser =
        [](fg::frame &stack,
           fg::json::const_iterator pos,
           fg::json::const_iterator end) {
            auto cnx = connect(stack);
            fg::json row;
            fostlib::insert(row, "reference", stack.lookup("odin.reference"));
            fostlib::insert(
                    row, "identity_id",
                    stack.resolve_string(stack.argument("username", pos, end)));
            fostlib::insert(row, "superuser", true);
            cnx.insert("odin.identity_superuser_ledger", row);
            cnx.commit();
            return fostlib::json();
        };


const fg::frame::builtin odin::lib::user = [](fg::frame &stack,
                                              fg::json::const_iterator pos,
                                              fg::json::const_iterator end) {
    auto cnx = connect(stack);
    auto username = stack.resolve_string(stack.argument("username", pos, end));
    auto ref = odin::reference();
    odin::create_user(cnx, ref, username);
    if (pos != end) {
        fostlib::log::warning(c_odin_fg, "Setting password is deprecated");
        auto password =
                stack.resolve_string(stack.argument("password", pos, end));
        odin::set_password(cnx, ref, username, username, password);
    }
    cnx.commit();
    return fostlib::json();
};


const fg::frame::builtin odin::lib::hash = [](fg::frame &stack,
                                              fg::json::const_iterator pos,
                                              fg::json::const_iterator end) {
    auto cnx = connect(stack);
    auto ref = odin::reference();
    auto username = stack.resolve_string(stack.argument("username", pos, end));
    auto hash = stack.resolve_string(stack.argument("hash", pos, end));
    auto process = stack.resolve(stack.argument("process", pos, end));
    odin::save_credential(cnx, ref, username, username, hash, process);
    cnx.commit();
    return fostlib::json();
};


const fg::frame::builtin odin::lib::expire = [](fg::frame &stack,
                                                fg::json::const_iterator pos,
                                                fg::json::const_iterator end) {
    auto cnx = connect(stack);
    auto username = stack.resolve_string(stack.argument("username", pos, end));
    fg::json user_values;
    fostlib::insert(user_values, "reference", stack.lookup("odin.reference"));
    fostlib::insert(user_values, "identity_id", username);
    fostlib::insert(user_values, "expires", fostlib::timestamp::now());
    cnx.insert("odin.identity_expiry_ledger", user_values);
    cnx.commit();
    return fostlib::json();
};
