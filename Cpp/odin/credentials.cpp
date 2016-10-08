/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/credentials.hpp>
#include <odin/odin.hpp>
#include <odin/pwhashproc.hpp>

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
            "odin.identity.expires < now() AS expired, "
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

    const auto hash = fostlib::coerce<fostlib::string>(
        user["credentials"]["password"]["hash"]);
    const auto process = user["credentials"]["password"]["process"];
    if ( check_password(password, hash, process) ) {
        if ( not fostlib::coerce<bool>(user["expired"]) ) {
            cnx.insert("odin.login_success", attempt);
            return user;
        } else {
            fostlib::log::warning(c_odin)
                ("", "Account expired")
                ("username", username)
                ("identity", user["identity"])
                ("credentials", user["credentials"]);
            return fostlib::json();
        }
    } else {
        fostlib::log::warning(c_odin)
            ("", "Password mismatch")
            ("username", username);
        cnx.insert("odin.login_failed", attempt);
        return fostlib::json();
    }
}


fostlib::jwt::mint odin::mint_jwt(const fostlib::json &user) {
    static const fostlib::jcursor subject("identity", "id");
    static const fostlib::jcursor full_name("identity", "full_name");
    static const fostlib::jcursor logout_count("credentials", "logout_count");

    fostlib::jwt::mint jwt(fostlib::sha256, odin::c_jwt_secret.value());
    jwt.subject(fostlib::coerce<fostlib::string>(user[subject]));

    if ( user.has_key(full_name) ) {
        jwt.claim("name", user[full_name]);
    }
    if ( user.has_key(logout_count) ) {
        jwt.claim(c_jwt_logout_claim.value(), user[logout_count]);
    }

    return jwt;
}

