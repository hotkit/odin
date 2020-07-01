/**
    Copyright 2018-2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <odin/google.hpp>
#include <odin/fg/native.hpp>
#include <odin/odin.hpp>

#include <fostgres/sql.hpp>
#include <fost/insert>
#include <fost/log>
#include <fost/ua/exceptions.hpp>


fostlib::json odin::google::get_user_detail(f5::u8view user_token) {
    fostlib::url base_url(
            fostlib::coerce<fostlib::string>("https://www.googleapis.com"));
    fostlib::url::filepath_string api{"/oauth2/v3/tokeninfo"};
    fostlib::url gg_url(base_url, api);
    gg_url.query().append("id_token", user_token);

    fostlib::json user_detail;
    try {
        user_detail =
                fostlib::ua::get_json(gg_url, fostlib::mime::mime_headers{});
        // Example user_detail :
        // https://developers.google.com/identity/sign-in/android/backend-auth
    } catch (fostlib::ua::http_error &e) {
        fostlib::log::error(c_odin)("Error", "get_user_detail")("URL", gg_url)(
                "status", e.data()["status-code"])("body", user_detail);
        throw fostlib::exceptions::not_implemented(
                __PRETTY_FUNCTION__, "Cannot retrieve user detail from google");
    }
    auto aud = fostlib::coerce<fostlib::string>(user_detail["aud"]);
    auto gg_aud = c_google_aud.value()["Client_ID"];
    for (const auto a : gg_aud) {
        if (aud == fostlib::coerce<fostlib::string>(a)) {
            fostlib::json gg_user;
            fostlib::insert(gg_user, "user_id", user_detail["sub"]);
            if (user_detail.has_key("name")) {
                fostlib::insert(gg_user, "name", user_detail["name"]);
            }
            if (user_detail.has_key("email")) {
                fostlib::insert(gg_user, "email", user_detail["email"]);
            }

            return gg_user;
        }
    }
    return fostlib::json();
}


fostlib::json odin::google::app_credentials(
        fostlib::pg::connection &cnx,
        const f5::u8view &user_id,
        const f5::u8view &app_id) {
    const fostlib::string sql(
            "SELECT "
            "odin.identity.tableoid AS identity__tableoid, "
            "odin.google_credentials.tableoid AS google_credentials__tableoid, "
            "odin.app_user.tableoid AS app_user__tableoid, "
            "odin.identity.*, odin.google_credentials.*, odin.app_user.* "
            "FROM odin.google_credentials "
            "JOIN odin.identity ON "
            "odin.identity.id=odin.google_credentials.identity_id "
            "LEFT JOIN odin.app_user ON "
            "odin.app_user.identity_id=odin.google_credentials.identity_id AND "
            "odin.app_user.app_id=$1 "
            "WHERE odin.google_credentials.google_user_id = $2");
    auto data = fostgres::sql(
            cnx, sql, std::vector<fostlib::string>{app_id, user_id});
    auto &rs = data.second;
    auto row = rs.begin();
    if (row == rs.end()) {
        fostlib::log::warning(c_odin)("", "Google user not found")(
                "google_user_id", user_id);
        return fostlib::json();
    }
    auto record = *row;
    if (++row != rs.end()) {
        fostlib::log::error(c_odin)("", "More than one google user returned")(
                "google_user_id", user_id);
        return fostlib::json();
    }

    fostlib::json user;
    for (std::size_t index{}; index < record.size(); ++index) {
        const auto parts = fostlib::split(data.first[index], "__");
        if (parts.size() && parts[parts.size() - 1] == "tableoid") continue;
        fostlib::jcursor pos;
        for (const auto &p : parts) pos /= p;
        fostlib::insert(user, pos, record[index]);
    }
    return user;
}


fostlib::json odin::google::credentials(
        fostlib::pg::connection &cnx, const f5::u8view &user_id) {
    static const fostlib::string sql(
            "SELECT "
            "odin.identity.tableoid AS identity__tableoid, "
            "odin.google_credentials.tableoid AS google_credentials__tableoid, "
            "odin.identity.*, odin.google_credentials.* "
            "FROM odin.google_credentials "
            "JOIN odin.identity ON "
            "odin.identity.id=odin.google_credentials.identity_id "
            "WHERE odin.google_credentials.google_user_id = $1");
    auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{user_id});
    auto &rs = data.second;
    auto row = rs.begin();
    if (row == rs.end()) {
        fostlib::log::warning(c_odin)("", "Google user not found")(
                "google_user_id", user_id);
        return fostlib::json();
    }
    auto record = *row;
    if (++row != rs.end()) {
        fostlib::log::error(c_odin)("", "More than one google user returned")(
                "google_user_id", user_id);
        return fostlib::json();
    }

    fostlib::json user;
    for (std::size_t index{}; index < record.size(); ++index) {
        const auto parts = fostlib::split(data.first[index], "__");
        if (parts.size() && parts[parts.size() - 1] == "tableoid") continue;
        fostlib::jcursor pos;
        for (const auto &p : parts) pos /= p;
        fostlib::insert(user, pos, record[index]);
    }
    return user;
}


void odin::google::set_google_credentials(
        fostlib::pg::connection &cnx,
        f5::u8view reference,
        f5::u8view identity_id,
        f5::u8view google_user_id) {
    fg::json user_values;
    fostlib::insert(user_values, "reference", reference);
    fostlib::insert(user_values, "identity_id", identity_id);
    fostlib::insert(user_values, "google_user_id", google_user_id);
    cnx.insert("odin.google_credentials_ledger", user_values);
}


std::optional<f5::u8string> odin::google::email_owner_identity_id(
        fostlib::pg::connection &cnx, fostlib::string email) {
    const f5::u8string sql(
            "SELECT oi.id FROM odin.identity oi INNER JOIN "
            "odin.google_credentials gc ON oi.id = gc.identity_id WHERE "
            "email=$1");
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