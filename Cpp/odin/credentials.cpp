/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/odin.hpp>
#include <odin/credentials.hpp>

#include <fost/insert>
#include <fost/log>
#include <fostgres/sql.hpp>


fostlib::json odin::credentials(
    fostlib::pg::connection &cnx,
    const fostlib::string &username, const fostlib::string &password,
    const fostlib::nullable<fostlib::host> &source,
    const fostlib::json &annotation
) {
    static const fostlib::string sql("SELECT "
            "odin.identity.tableoid AS identity__tableoid, "
            "odin.credentials.tableoid AS credentials__tableoid, "
            "odin.identity.*, odin.credentials.* "
        "FROM odin.credentials "
        "JOIN odin.identity ON odin.identity.id=odin.credentials.identity_id "
        "WHERE odin.credentials.login = $1");

    fostlib::json attempt;
    fostlib::insert(attempt, "username", username);
    fostlib::insert(attempt, "source_address", source);
    fostlib::insert(attempt, "annotation", annotation);

    auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{username});
    auto &rs = data.second;
    auto row = rs.begin();
    if ( row == rs.end() ) {
        fostlib::log::warning(c_odin)
            ("", "User not found")
            ("username", username);
        cnx.insert("odin.login_failed", attempt);
        return fostlib::json();
    }
    auto record = *row;
    if ( ++row != rs.end() ) {
        fostlib::log::error(c_odin)
            ("", "More than one user returned")
            ("username", username);
        cnx.insert("odin.login_failed", attempt);
        return fostlib::json();
    }

    fostlib::json user;
    for ( std::size_t index{0}; index < record.size(); ++index ) {
        const auto parts = fostlib::split(data.first[index], "__");
        if ( parts.size() && parts[parts.size() - 1] == "tableoid" )
            continue;
        fostlib::jcursor pos;
        for ( const auto &p : parts ) pos /= p;
        fostlib::insert(user, pos, record[index]);
    }

    if ( user["credentials"]["password"]["hash"] == fostlib::json(password) ) {
        cnx.insert("odin.login_success", attempt);
        return user;
    } else {
        fostlib::log::warning(c_odin)
            ("", "Password mismatch")
            ("username", username);
        cnx.insert("odin.login_failed", attempt);
        return fostlib::json();
    }
}

