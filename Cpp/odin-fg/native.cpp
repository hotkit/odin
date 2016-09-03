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


namespace {


    fostlib::pg::connection connect(fg::frame &stack) {
        fostlib::pg::connection cnx(stack.lookup("pg.dsn"));
        odin::reference(cnx, stack.resolve_string(stack.lookup("odin.reference")));
        return cnx;
    }


    fg::json sql_file(
        fg::frame &stack, fg::json::const_iterator pos, fg::json::const_iterator end
    ) {
        auto sql = fostlib::coerce<fostlib::utf8_string>(
            fostlib::utf::load_file(
                fostlib::coerce<boost::filesystem::path>(
                    stack.resolve_string(stack.argument("filename", pos, end)))));
        auto cnx = connect(stack);
        cnx.exec(sql);
        cnx.commit();
        return fostlib::json();
    }


    fg::json user(
        fg::frame &stack, fg::json::const_iterator pos, fg::json::const_iterator end
    ) {
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
    }


    const fg::register_builtins g_odin(
        [](fg::frame &stack) {
            stack.symbols["odin.reference"] = odin::reference();
            stack.native["odin.jwt.authorization"] = odin::lib::jwt;
            stack.native["odin.sql.file"] = sql_file;
            stack.native["odin.user"] = user;
        });


}

