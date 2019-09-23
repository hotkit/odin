/**
    Copyright 2018-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */

#include <odin/facebook.hpp>
#include <odin/fg/native.hpp>
#include <odin/odin.hpp>

#include <fost/http>
#include <fost/log>
#include <fostgres/sql.hpp>

#include <fost/insert>

namespace {


    std::unique_ptr<fostlib::http::user_agent::response> get_or_mock(
            fostlib::http::user_agent &ua,
            fostlib::url url,
            fostlib::json config = {}) {
        if (config.isnull()) { return ua.get(url); }
        /// Create mock response based on config
        int status{200};
        if (config.has_key("status")) {
            status = fostlib::coerce<int>(config["status"]);
        }
        fostlib::json body{};
        if (config.has_key("body")) { body = config["body"]; }
        auto const bodydata = fostlib::json::unparse(body, false);
        // TODO: Can set headers
        fostlib::mime::mime_headers headers;
        // fostlib::json headers{};
        return std::make_unique<fostlib::http::user_agent::response>(
                "GET", url, 200,
                std::make_unique<fostlib::binary_body>(
                        bodydata.memory().begin(), bodydata.memory().end(),
                        headers));
    }
}

fostlib::json odin::facebook::get_user_detail(
        fostlib::pg::connection &cnx,
        f5::u8view user_token,
        fostlib::json config) {

    fostlib::http::user_agent ua{};
    fostlib::url base_facebook_url(odin::c_facebook_endpoint.value());

    /// Get list of ids for business from Facebook
    fostlib::url ids_for_biz_url(base_facebook_url, "/me/ids_for_business");
    fostlib::url::query_string ids_for_biz_qs{};
    ids_for_biz_qs.append("access_token", user_token);
    ids_for_biz_url.query(ids_for_biz_qs);
    fostlib::json ids_for_biz_conf{};
    if (config.has_key(fostlib::jcursor{"facebook-mock", "ids_for_business"})) {
        ids_for_biz_conf =
                config[fostlib::jcursor{"facebook-mock", "ids_for_business"}];
    }
    auto const ids_for_biz_resp =
            get_or_mock(ua, ids_for_biz_url, ids_for_biz_conf);
    fostlib::json const ids_for_biz =
            fostlib::json::parse(fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(
                            ids_for_biz_resp->body()->data())));
    if (!ids_for_biz.has_key("data")) {
        fostlib::log::error(c_odin)("Error", "ids_for_business")(
                "URL", ids_for_biz_url)("status", ids_for_biz_resp->status())(
                "body", ids_for_biz);
        // TODO: Should return 422
        throw fostlib::exceptions::not_implemented(
                __PRETTY_FUNCTION__,
                "Cannot retrieve /me/ids_for_business from Facebook");
    }

    fostlib::json fb_user{};
    /// Check with allow facebook apps
    auto const fb_conf = odin::c_facebook_apps.value();
    auto const allow_apps = fb_conf["allowed"];
    fostlib::json fb_app_id{};
    bool found_app_id = false;
    std::optional<f5::u8string> userid;
    fostlib::jcursor const app_id_cursor{"app", "id"};
    std::for_each(
            ids_for_biz["data"].begin(), ids_for_biz["data"].end(),
            [&](auto appdata) {
                if (!found_app_id) {
                    auto const app_id = appdata[app_id_cursor];
                    auto const usr_id =
                            fostlib::coerce<f5::u8string>(appdata["id"]);
                    if (std::find(allow_apps.begin(), allow_apps.end(), app_id)
                        != allow_apps.end()) {
                        userid = usr_id;
                        auto data = fostgres::sql(
                                cnx,
                                "SELECT * FROM odin.facebook_credentials WHERE "
                                "facebook_user_id=$1",
                                std::vector<fostlib::string>{usr_id});
                        auto &rs = data.second;
                        if (rs.begin() != rs.end()) { found_app_id = true; }
                    }
                }
            });

    if (!userid.has_value()) {
        // The allowed apps are not in the list
        // TODO: Should return 422
        throw fostlib::exceptions::not_implemented(
                __PRETTY_FUNCTION__,
                "Authorized facebook apps not found for this access "
                "token");
    }

    fostlib::insert(fb_user, "id", static_cast<f5::u8view>(userid.value()));

    /// Retrieve facebook user detail
    fostlib::url user_detail_url(base_facebook_url, "/me");
    fostlib::url::query_string user_detail_qs{};
    user_detail_qs.append("access_token", user_token);
    user_detail_qs.append("fields", "name,email");
    user_detail_url.query(user_detail_qs);
    fostlib::json me_conf{};
    if (config.has_key(fostlib::jcursor{"facebook-mock", "me"})) {
        me_conf = config[fostlib::jcursor{"facebook-mock", "me"}];
    }
    auto const user_detail_resp = get_or_mock(ua, user_detail_url, me_conf);
    auto user_detail = fostlib::json::parse(fostlib::coerce<fostlib::string>(
            fostlib::coerce<fostlib::utf8_string>(
                    user_detail_resp->body()->data())));
    fostlib::log::error(c_odin)("Response", user_detail);
    if (user_detail.has_key("error")) {
        fostlib::log::error(c_odin)("Error", "get-user-detail")(
                "URL", user_detail_url)("status", user_detail_resp->status())(
                "body", user_detail);
        // TODO: Should return 422
        throw fostlib::exceptions::not_implemented(
                __PRETTY_FUNCTION__,
                "Cannot retrieve /me?field=name,email from Facebook");
    }
    if (user_detail.has_key("name")) {
        fostlib::insert(fb_user, "name", user_detail["name"]);
    }
    if (user_detail.has_key("email")) {
        fostlib::insert(fb_user, "email", user_detail["email"]);
    }
    return fb_user;
}


fostlib::json odin::facebook::app_credentials(
        fostlib::pg::connection &cnx, const f5::u8view &user_id, const f5::u8view &app_id) {
    static const fostlib::string sql(
            "SELECT "
            "odin.identity.tableoid AS identity__tableoid, "
            "odin.facebook_credentials.tableoid AS "
            "facebook_credentials__tableoid, "
            "odin.app_user.tableoid AS app_user__tableoid, "
            "odin.identity.*, odin.facebook_credentials.*, odin.app_user.* "
            "FROM odin.facebook_credentials "
            "JOIN odin.identity ON "
            "odin.identity.id=odin.facebook_credentials.identity_id "
            "LEFT JOIN odin.app_user ON "
            "odin.app_user.identity_id=odin.facebook_credentials.identity_id AND "
            "odin.app_user.app_id=$1 "
            "WHERE odin.facebook_credentials.facebook_user_id = $2");
    auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{app_id, user_id});
    auto &rs = data.second;
    auto row = rs.begin();
    if (row == rs.end()) {
        fostlib::log::warning(c_odin)("", "Facebook user not found")(
                "facebook_user_id", user_id);
        return fostlib::json();
    }
    auto record = *row;
    if (++row != rs.end()) {
        fostlib::log::error(c_odin)("", "More than one facebook user returned")(
                "facebook_user_id", user_id);
        return fostlib::json();
    }

    fostlib::json user;
    for (std::size_t index{0}; index < record.size(); ++index) {
        const auto parts = fostlib::split(data.first[index], "__");
        if (parts.size() && parts[parts.size() - 1] == "tableoid") continue;
        fostlib::jcursor pos;
        for (const auto &p : parts) pos /= p;
        fostlib::insert(user, pos, record[index]);
    }

    return user;
}


fostlib::json odin::facebook::credentials(
        fostlib::pg::connection &cnx, const f5::u8view &user_id) {
    static const fostlib::string sql(
            "SELECT "
            "odin.identity.tableoid AS identity__tableoid, "
            "odin.facebook_credentials.tableoid AS "
            "facebook_credentials__tableoid, "
            "odin.identity.*, odin.facebook_credentials.* "
            "FROM odin.facebook_credentials "
            "JOIN odin.identity ON "
            "odin.identity.id=odin.facebook_credentials.identity_id "
            "WHERE odin.facebook_credentials.facebook_user_id = $1");
    auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{user_id});
    auto &rs = data.second;
    auto row = rs.begin();
    if (row == rs.end()) {
        fostlib::log::warning(c_odin)("", "Facebook user not found")(
                "facebook_user_id", user_id);
        return fostlib::json();
    }
    auto record = *row;
    if (++row != rs.end()) {
        fostlib::log::error(c_odin)("", "More than one facebook user returned")(
                "facebook_user_id", user_id);
        return fostlib::json();
    }

    fostlib::json user;
    for (std::size_t index{0}; index < record.size(); ++index) {
        const auto parts = fostlib::split(data.first[index], "__");
        if (parts.size() && parts[parts.size() - 1] == "tableoid") continue;
        fostlib::jcursor pos;
        for (const auto &p : parts) pos /= p;
        fostlib::insert(user, pos, record[index]);
    }

    return user;
}


void odin::facebook::set_facebook_credentials(
        fostlib::pg::connection &cnx,
        f5::u8view reference,
        f5::u8view identity_id,
        f5::u8view facebook_user_id) {
    fg::json user_values;
    fostlib::insert(user_values, "reference", reference);
    fostlib::insert(user_values, "identity_id", identity_id);
    fostlib::insert(user_values, "facebook_user_id", facebook_user_id);
    cnx.insert("odin.facebook_credentials_ledger", user_values);
}
