/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/fg/native.hpp>
#include <odin/nonce.hpp>
#include <odin/pwhashproc.hpp>

#include <fost/insert>


const fg::frame::builtin odin::lib::user =
    [](fg::frame &stack, fg::json::const_iterator pos, fg::json::const_iterator end) {
        auto cnx = connect(stack);
        auto username = stack.resolve_string(stack.argument("username", pos, end));
        fg::json user_values;
        fostlib::insert(user_values, "reference", stack.lookup("odin.reference"));
        fostlib::insert(user_values, "identity_id", username);
        cnx.insert("odin.identity_ledger", user_values);
        if ( pos != end ) {
            auto password = stack.resolve_string(stack.argument("password", pos, end));
            auto hashed = odin::set_password(password);
            fostlib::insert(user_values, "password", hashed.first);
            fostlib::insert(user_values, "process", hashed.second);
            cnx.insert("odin.credentials_password_ledger", user_values);
        }
        cnx.commit();
        return fostlib::json();
    };

