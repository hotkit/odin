/**
    Copyright 2018 Felspar Co Ltd. <http://odin.felspar.com/>

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
        f5::u8view user_token, fostlib::json config) {

    fostlib::http::user_agent ua{};
    fostlib::url base_facebook_url(odin::c_facebook_endpoint.value());

    /// Get list of ids for business from Facebook
    fostlib::url ids_for_biz_url(base_facebook_url, "/me/ids_for_business");
    fostlib::url::query_string qs{};
    qs.append("access_token", user_token);
    ids_for_biz_url.query(qs);
    auto const ids_for_biz_resp = get_or_mock(
            ua, ids_for_biz_url,
            config[fostlib::jcursor{"facebook-mock", "ids_for_business"}]);
    fostlib::json const ids_for_biz =
            fostlib::json::parse(fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(
                            ids_for_biz_resp->body()->data())));
    if (ids_for_biz.has_key("error")) {
        fostlib::log::error(c_odin)("Error", "ids_for_business")(
                "URL", ids_for_biz_url)("status", ids_for_biz_resp->status())(
                "body", ids_for_biz);
        throw fostlib::exceptions::not_implemented(
                __PRETTY_FUNCTION__,
                "Cannot retrieve /me/ids_for_business from Facebook");
    }
    /// Check with allow facebook apps
    auto const fb_conf = odin::c_facebook_apps.value();
    auto const allow_apps = fb_conf["allowed"];
    auto const main_app = fb_conf["main"];
    for (const auto allow_app : allow_apps) {}


    // user_agent ua;
    // if (configuration.has_key("ids_for_business")) {
    //     idsforbiz=make_shared<user_agent::response>(status,
    //     configuration["ids_for_business"]["body"])
    // } else {
    //     // get_or_mock(&user_agent&, url, config)
    //     idsforbiz=ua.get_or_mock(ids_forbiz_url,
    //     configuration["ids_for_business"]);
    // }

    // // Loop through and check app ID and hot now app ID and find HN user ID
    // user_id = ....;

    // if(configuraiton.has_key("me")) {
    //     me = configuration["me"];
    // } else {
    //     me = ua.get("/me?...");
    // }

    // Process the user data to save to DB\


    // return user;

    // for (const auto fb_app : fb_apps) {
    //     const auto app_token = get_app_token(
    //             fostlib::coerce<f5::u8string>(fb_app["app_id"]),
    //             fostlib::coerce<f5::u8string>(fb_app["app_secret"]));
    //     if (is_user_authenticated(app_token, user_token)) {
    //         fostlib::url fb_url(base_url, api);
    //         fostlib::url::query_string qs{};
    //         qs.append("access_token", user_token);
    //         qs.append("fields", "id,name,email");
    //         fb_url.query(qs);
    //         // move the user agent to be another fn
    //         // check config, if has mock, return the mock
    // fostlib::http::user_agent ua(fb_url);
    // auto response = ua.get(fb_url);
    //         auto response_data = fostlib::coerce<fostlib::string>(
    //                 fostlib::coerce<fostlib::utf8_string>(
    //                         response->body()->data()));
    //         return fostlib::json::parse(response_data);
    //     }
    // }
    return fostlib::json{};
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
