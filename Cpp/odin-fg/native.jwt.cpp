/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/fg/native.hpp>


namespace {


    fg::json jwt(
        fg::frame &stack, fg::json::const_iterator pos, fg::json::const_iterator end
    ) {
        return fg::json();
    }


}


const fg::frame::builtin odin::lib::jwt = ::jwt;
