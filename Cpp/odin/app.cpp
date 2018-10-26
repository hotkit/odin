/**
    Copyright 2016-2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/

#include <fostgres/sql.hpp>

#include <odin/app.hpp>
#include <odin/fg/native.hpp>
#include <odin/odin.hpp>

#include <fost/insert>
#include <fost/log>
#include <fostgres/sql.hpp>


fostlib::json odin::app::get_detail(
    fostlib::pg::connection &cnx,
    const f5::u8view app_id
) {
    static const f5::u8view sql("SELECT "
        "odin.app.tableoid AS app__tableoid, odin.app.* "
        "FROM odin.app "
        "WHERE odin.app.app_id = $1");

    auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{app_id});
    auto &rs = data.second;
    auto row = rs.begin();
    if ( row == rs.end() ) {
        fostlib::log::warning(c_odin)("", "App not found")("app_id", app_id);
        return fostlib::json();
    }
    auto record = *row;
    fostlib::json app;
    for ( std::size_t index{0}; index < record.size(); ++index ) {
        const auto parts = fostlib::split(data.first[index], "__");
        if ( parts.size() && parts[parts.size() - 1] == "tableoid" )
            continue;
        fostlib::jcursor pos;
        for ( const auto &p : parts ) pos /= p;
        fostlib::insert(app, pos, record[index]);
    }
    return app;
}


fostlib::jwt::mint odin::app::mint_user_jwt(
    const fostlib::json &user, const fostlib::json &app, fostlib::json payload
) {
    static const fostlib::jcursor subject("identity", "id");
    static const fostlib::jcursor app_id("app", "app_id");
    const fostlib::string jwt_secret = odin::c_jwt_secret.value()
            + fostlib::coerce<fostlib::string>(app[app_id]);
    fostlib::jwt::mint jwt{fostlib::sha256, jwt_secret, std::move(payload)};
    jwt.subject(fostlib::coerce<fostlib::string>(user[subject]));
    jwt.claim("iss", app[app_id]);
    return jwt;
}


void odin::app::save_app_user(
    fostlib::pg::connection &cnx, f5::u8view reference,
    const f5::u8view identity_id, const f5::u8view app_id
) {
    fg::json app_user_values;
    fostlib::insert(app_user_values, "reference", reference);
    fostlib::insert(app_user_values, "identity_id", identity_id);
    fostlib::insert(app_user_values, "app_id", app_id);
    cnx.insert("odin.app_user_ledger", app_user_values);
}


fostlib::json odin::app::get_app_user(
    fostlib::pg::connection &cnx,
    const f5::u8view app_id,
    const f5::u8view identity_id
) {
    // TODO: Check app access policy, now support only OPEN
    static const f5::u8view sql("SELECT "
        "odin.app_user.tableoid AS app__tableoid, odin.app_user.* "
        "FROM odin.app_user "
        "WHERE odin.app_user.app_id = $1 "
        "AND odin.app_user.identity_id = $2");

    auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{app_id, identity_id});
    auto &rs = data.second;
    auto row = rs.begin();
    if ( row == rs.end() ) {
        fostlib::log::warning(c_odin)("", "User or App not found")("app_id", app_id)("identity_id", identity_id);
        return fostlib::json();
    }
    auto record = *row;
    fostlib::json app_user;
    for ( std::size_t index{0}; index < record.size(); ++index ) {
        const auto parts = fostlib::split(data.first[index], "__");
        if ( parts.size() && parts[parts.size() - 1] == "tableoid" )
            continue;
        fostlib::jcursor pos;
        for ( const auto &p : parts ) pos /= p;
        fostlib::insert(app_user, pos, record[index]);
    }
    return app_user;
}