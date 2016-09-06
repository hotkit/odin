/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/fg/native.hpp>
#include <odin/odin.hpp>

#include <fost/crypto>
#include <fost/insert>


namespace {


    fg::json jwt(
        fg::frame &stack, fg::json::const_iterator pos, fg::json::const_iterator end
    ) {
        auto username = stack.resolve_string(stack.argument("username", pos, end));
        fostlib::jwt::mint jwt(fostlib::sha256, odin::c_jwt_secret.value());
        jwt.subject(username);

        auto token = jwt.token();
        stack.symbols["odin.jwt"] = token;

        auto headers = stack.symbols["testserver.headers"];
        fostlib::insert(headers, "Authorization", fg::json("Bearer " + token));
        stack.symbols["testserver.headers"] = headers;

        return fg::json(std::move(token));
    }


}


const fg::frame::builtin odin::lib::jwt = ::jwt;
