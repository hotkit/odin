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

#include <fost/datetime>
#include <fost/insert>


const fg::frame::builtin odin::lib::superuser =
    [](fg::frame &stack, fg::json::const_iterator pos, fg::json::const_iterator end) {
        auto cnx = connect(stack);
        fg::json row;
        fostlib::insert(row, "reference", stack.lookup("odin.reference"));
        fostlib::insert(row, "identity_id",
            stack.resolve_string(stack.argument("username", pos, end)));
        fostlib::insert(row, "superuser", true);
        cnx.insert("odin.identity_superuser_ledger", row);
        cnx.commit();
        return fostlib::json();
    };


const fg::frame::builtin odin::lib::user =
    [](fg::frame &stack, fg::json::const_iterator pos, fg::json::const_iterator end) {
        auto cnx = connect(stack);
        auto username = stack.resolve_string(stack.argument("username", pos, end));
        if ( pos != end ) {
            auto password = stack.resolve_string(stack.argument("password", pos, end));
            return odin::create_user(cnx, username, password);
        }
        return odin::create_user(cnx, username);
    };


const fg::frame::builtin odin::lib::expire =
    [](fg::frame &stack, fg::json::const_iterator pos, fg::json::const_iterator end) {
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

