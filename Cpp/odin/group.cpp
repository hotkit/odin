/**
    Copyright 2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/

#include <fostgres/sql.hpp>

#include <odin/group.hpp>
#include <odin/fg/native.hpp>
#include <odin/pwhashproc.hpp>

#include <fost/insert>


void odin::group::alter_membership (
    fostlib::pg::connection &cnx,
    f5::u8view reference,
    f5::u8view identity_id,
    f5::u8view group_slug,
    bool is_member
) {
    fg::json group_membership;
    fostlib::insert(group_membership, "reference", reference);
    fostlib::insert(group_membership, "identity_id", identity_id);
    fostlib::insert(group_membership, "group_slug", group_slug);
    fostlib::insert(group_membership, "member", is_member);
    cnx.insert("odin.group_membership_ledger", group_membership);
}
