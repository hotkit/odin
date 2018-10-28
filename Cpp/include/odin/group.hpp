/**
    Copyright 2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#pragma once


#include <fost/postgres>


namespace odin {


    namespace group {


    /// Alter membership of group, does not commit transaction
    void alter_membership(fostlib::pg::connection &cnx, f5::u8view reference, f5::u8view identity_id,
        f5::u8view group_slug, bool is_member);

    /// Add membership to group, does not commit transaction
    void add_membership(fostlib::pg::connection &cnx, f5::u8view reference, f5::u8view identity_id,
        f5::u8view group_slug);

    /// Remove membership to group, does not commit transaction
    void remove_membership(fostlib::pg::connection &cnx, f5::u8view reference, f5::u8view identity_id,
        f5::u8view group_slug);


    }


}
