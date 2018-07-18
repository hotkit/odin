/**
    Copyright 2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/

#include <odin/google.hpp>
#include <odin/fg/native.hpp>
#include <odin/odin.hpp>

#include <fost/http>
#include <fost/log>
#include <fostgres/sql.hpp>

#include <fost/insert>


fostlib::json odin::google::get_user_detail(f5::u8view user_token) {
    fostlib::url base_url(fostlib::coerce<fostlib::string>("https://www.googleapis.com"));
    fostlib::url::filepath_string api{"/oauth2/v3/tokeninfo"};
    fostlib::url gg_url(base_url, api);
    fostlib::url::query_string qs{};
    qs.append("id_token", user_token);
    gg_url.query(qs);
    fostlib::http::user_agent ua(gg_url);
    auto response = ua.get(gg_url);

    // Log details
    fostlib::json rj;
    fostlib::insert(rj, "status", response->status());
    // fostlib::insert(rj, "body", "size", response->body()->data().size());
    // fostlib::insert(rj, "body", "data",
    //     fostlib::coerce<fostlib::string>(fostlib::coerce<fostlib::utf8_string>(response->body()->data())));
    fostlib::insert(rj, "headers", response->headers());
    fostlib::log::warning(c_odin)("response", rj);
    return fostlib::json();
    if ( response->status() == 400 )
        return fostlib::json();
    auto response_data = fostlib::coerce<fostlib::string>(fostlib::coerce<fostlib::utf8_string>(response->body()->data()));
    fostlib::json body = fostlib::json::parse(response_data);
    fostlib::log::warning(c_odin)
            ("response_data", response_data);
    auto aud = fostlib::coerce<fostlib::string>(body["aud"]);
    // if ( aud != odin::c_google_aud.value())
        // return fostlib::json();
    return body;
}


fostlib::json odin::google::credentials(fostlib::pg::connection &cnx, const f5::u8view &user_id) {
    static const fostlib::string sql("SELECT "
            "odin.identity.tableoid AS identity__tableoid, "
            "odin.google_credentials.tableoid AS google_credentials__tableoid, "
            "odin.identity.*, odin.google_credentials.* "
        "FROM odin.google_credentials "
        "JOIN odin.identity ON odin.identity.id=odin.google_credentials.identity_id "
        "WHERE odin.google_credentials.google_user_id = $1");
    auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{user_id});
    auto &rs = data.second;
    auto row = rs.begin();
    if ( row == rs.end() ) {
        fostlib::log::warning(c_odin)
            ("", "Google user not found")
            ("google_user_id", user_id);
        return fostlib::json();
    }
    auto record = *row;
    if ( ++row != rs.end() ) {
        fostlib::log::error(c_odin)
            ("", "More than one google user returned")
            ("google_user_id", user_id);
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
    return user;
}


void odin::google::set_google_credentials(
    fostlib::pg::connection &cnx, f5::u8view reference, f5::u8view identity_id, f5::u8view google_user_id
) {
    fg::json user_values;
    fostlib::insert(user_values, "reference", reference);
    fostlib::insert(user_values, "identity_id", identity_id);
    fostlib::insert(user_values, "google_user_id", google_user_id);
    cnx.insert("odin.google_credentials_ledger", user_values);
}