/**
    Copyright 2016-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <odin/fg/native.hpp>

#include <fost/insert>


const fg::frame::builtin odin::lib::assign = [](fg::frame &stack,
                                                fg::json::const_iterator pos,
                                                fg::json::const_iterator end) {
    auto cnx = connect(stack);
    fg::json row;
    fostlib::insert(row, "reference", stack.lookup("odin.reference"));
    fostlib::insert(row, "allows", true);
    fostlib::insert(
            row, "group_slug",
            stack.resolve_string(stack.argument("group", pos, end)));
    fostlib::insert(
            row, "permission_slug",
            stack.resolve_string(stack.argument("permission", pos, end)));
    cnx.insert("odin.group_grant_ledger", row);
    while (pos != end) {
        fostlib::jcursor("permission_slug")
                .replace(
                        row,
                        stack.resolve_string(
                                stack.argument("permission", pos, end)));
        cnx.insert("odin.group_grant_ledger", row);
    }
    cnx.commit();
    return fostlib::json();
};


const fg::frame::builtin odin::lib::group = [](fg::frame &stack,
                                               fg::json::const_iterator pos,
                                               fg::json::const_iterator end) {
    auto cnx = connect(stack);
    fg::json row;
    fostlib::insert(row, "reference", stack.lookup("odin.reference"));
    fostlib::insert(
            row, "group_slug",
            stack.resolve_string(stack.argument("name", pos, end)));
    if (pos != end) {
        fostlib::insert(
                row, "description",
                stack.resolve_string(stack.argument("description", pos, end)));
    }
    cnx.insert("odin.group_ledger", row);
    cnx.commit();
    return fostlib::json();
};


const fg::frame::builtin odin::lib::membership =
        [](fg::frame &stack,
           fg::json::const_iterator pos,
           fg::json::const_iterator end) {
            auto cnx = connect(stack);
            fg::json row;
            fostlib::insert(row, "reference", stack.lookup("odin.reference"));
            fostlib::insert(row, "member", true);
            fostlib::insert(
                    row, "identity_id",
                    stack.resolve_string(stack.argument("user", pos, end)));
            fostlib::insert(
                    row, "group_slug",
                    stack.resolve_string(stack.argument("group", pos, end)));
            cnx.insert("odin.group_membership_ledger", row);
            while (pos != end) {
                fostlib::jcursor("group_slug")
                        .replace(
                                row,
                                stack.resolve_string(
                                        stack.argument("group", pos, end)));
                cnx.insert("odin.group_membership_ledger", row);
            }
            cnx.commit();
            return fostlib::json();
        };
