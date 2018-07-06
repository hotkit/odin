/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/fg/native.hpp>
#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/pwhashproc.hpp>

#include <fost/insert>


const fostlib::module odin::c_odin_fg(odin::c_odin, "fg");


namespace {


    fg::json sql_file(
        fg::frame &stack, fg::json::const_iterator pos, fg::json::const_iterator end
    ) {
        auto sql = fostlib::coerce<fostlib::utf8_string>(
            fostlib::utf::load_file(
                fostlib::coerce<boost::filesystem::path>(
                    stack.resolve_string(stack.argument("filename", pos, end)))));
        auto cnx = odin::connect(stack);
        cnx.exec(sql);
        cnx.commit();
        return fostlib::json();
    }


    const fg::register_builtins g_odin(
        [](fg::frame &stack) {
            stack.symbols["odin.reference"] = odin::reference();
            stack.native["odin.assign"] = odin::lib::assign;
            stack.native["odin.group"] = odin::lib::group;
            stack.native["odin.jwt.authorization"] = odin::lib::jwt;
            stack.native["odin.jwt.payload"] = odin::lib::jwt_payload;
            stack.native["odin.membership"] = odin::lib::membership;
            stack.native["odin.permission"] = odin::lib::permission;
            stack.native["odin.sql.file"] = sql_file;
            stack.native["odin.superuser"] = odin::lib::superuser;
            stack.native["odin.user"] = odin::lib::user;
            stack.native["odin.user.expire"] = odin::lib::expire;
        });


}


fostlib::pg::connection odin::connect(fg::frame &stack) {
    fostlib::pg::connection cnx(stack.lookup("pg.dsn"));
    odin::reference(cnx, stack.resolve_string(stack.lookup("odin.reference")));
    return cnx;
}

