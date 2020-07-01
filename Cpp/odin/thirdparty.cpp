/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */

#include <odin/thirdparty.hpp>
#include <odin/app.hpp>
#include <odin/fg/native.hpp>
#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/user.hpp>

#include <fost/core>
#include <fost/log>
#include <fost/http>
#include <fost/postgres>
#include <fostgres/sql.hpp>
#include <fost/urlhandler>

#include <pqxx/except>


std::optional<f5::u8string> odin::thirdparty::email_owner_identity_id(
        fostlib::pg::connection &cnx, fostlib::string email) {
    const f5::u8string sql(
            "SELECT oi.id FROM odin.identity oi "
            "LEFT JOIN odin.facebook_credentials fc "
            "ON oi.id = fc.identity_id "
            "LEFT JOIN odin.google_credentials gg "
            "ON oi.id = gg.identity_id "
            "LEFT JOIN odin.apple_credentials ap "
            "ON oi.id = ap.identity_id "
            "WHERE email=$1 "
            "AND (fc.identity_id IS NOT NULL "
            "OR gg.identity_id IS NOT NULL "
            "OR ap.identity_id IS NOT NULL);");
    auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{email});
    auto &rs = data.second;
    auto row = rs.begin();
    if (row == rs.end()) { return {}; }
    if (++row != rs.end()) {
        fostlib::log::error(c_odin)("", "More than one email owner returned")(
                "email", email);
        return {};
    }
    return fostlib::coerce<f5::u8string>((*row)[std::size_t{0}]);
}
