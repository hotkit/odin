/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <odin/apple.hpp>
#include <odin/fg/native.hpp>
#include <odin/odin.hpp>

#include <fostgres/sql.hpp>
#include <fost/crypto>
#include <fost/http>
#include <fost/insert>
#include <fost/log>
#include <fost/ua/exceptions.hpp>


fostlib::json odin::apple::get_user_detail(f5::u8view user_token) {

    fostlib::url apple_key_url("https://appleid.apple.com/auth/keys");
    // Get RSA public key from apple
    fostlib::json apple_pubic_keys =
            fostlib::ua::get_json(apple_key_url, fostlib::mime::mime_headers{});

    // Decode jwt use public key with correct kid
    const auto parts = fostlib::split(user_token, ".");
    if (parts.size() != 3u) return fostlib::json{};
    const fostlib::base64_string b64_header{parts[0]};
    const auto v64_header =
            fostlib::coerce<std::vector<unsigned char>>(b64_header);
    const auto u8_header = fostlib::coerce<fostlib::utf8_string>(v64_header);
    const auto str_header = fostlib::coerce<fostlib::string>(u8_header);
    const auto header = fostlib::json::parse(str_header);

    const auto public_key_kid = header["kid"];
    fostlib::json matched_key{};
    std::any_of(
            apple_pubic_keys["keys"].begin(), apple_pubic_keys["keys"].end(),
            [&](auto key) {
                if (key["kid"] == public_key_kid) {
                    matched_key = key;
                    return true;
                }
                return false;
            });

    if (matched_key.isnull()) {
        fostlib::log::error(c_odin)("public_key_kid", public_key_kid)(
                "apple_pubic_keys", apple_pubic_keys);
        throw fostlib::exceptions::not_implemented(
                __PRETTY_FUNCTION__,
                "No apple public key match with header kid");
    }

    auto key_n = fostlib::json::unparse(matched_key["n"], false);
    auto key_e = fostlib::json::unparse(matched_key["e"], false);
    std::vector<f5::byte> byte_n(key_n.begin(), key_n.end());
    std::vector<f5::byte> byte_e(key_e.begin(), key_e.end());
    byte_e.insert(byte_e.end(), f5::byte(0x00));
    byte_e.insert(byte_e.end(), byte_n.begin(), byte_n.end());

    auto const jwt = fostlib::jwt::token::load(
            user_token,
            [&byte_e](auto, auto) -> std::vector<f5::byte> { return byte_e; });

    if (not jwt) {
        fostlib::log::error(c_odin)("Invalid apple token", "verify failed");
        throw fostlib::exceptions::not_implemented(
                __PRETTY_FUNCTION__, "Invalid apple token");
    }

    fostlib::json user_detail = jwt.value().payload;

    if (user_detail["aud"] != c_apple_aud.value()["aud"]) {
        fostlib::log::error(c_odin)("apple token aud", user_detail["aud"])(
                "app aud", c_apple_aud.value()["aud"]);
        throw fostlib::exceptions::not_implemented(
                __PRETTY_FUNCTION__, "Invalid apple token aud");
    }

    fostlib::json apple_user;
    fostlib::insert(apple_user, "user_id", user_detail["sub"]);
    if (user_detail.has_key("email")) {
        fostlib::insert(apple_user, "email", user_detail["email"]);
    }
    return apple_user;
}


fostlib::json odin::apple::app_credentials(
        fostlib::pg::connection &cnx,
        const f5::u8view &user_id,
        const f5::u8view &app_id) {
    const fostlib::string sql(
            "SELECT "
            "odin.identity.tableoid AS identity__tableoid, "
            "odin.apple_credentials.tableoid AS "
            "apple_credentials__tableoid, "
            "odin.app_user.tableoid AS app_user__tableoid, "
            "odin.identity.*, odin.apple_credentials.*, odin.app_user.* "
            "FROM odin.apple_credentials "
            "JOIN odin.identity ON "
            "odin.identity.id=odin.apple_credentials.identity_id "
            "LEFT JOIN odin.app_user ON "
            "odin.app_user.identity_id=odin.apple_credentials.identity_id "
            "AND "
            "odin.app_user.app_id=$1 "
            "WHERE odin.apple_credentials.apple_user_id = $2");
    auto data = fostgres::sql(
            cnx, sql, std::vector<fostlib::string>{app_id, user_id});
    auto &rs = data.second;
    auto row = rs.begin();
    if (row == rs.end()) {
        fostlib::log::warning(c_odin)("", "apple user not found")(
                "apple_user_id", user_id);
        return fostlib::json();
    }
    auto record = *row;
    if (++row != rs.end()) {
        fostlib::log::error(c_odin)("", "More than one apple user returned")(
                "apple_user_id", user_id);
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


void odin::apple::set_apple_credentials(
        fostlib::pg::connection &cnx,
        f5::u8view reference,
        f5::u8view identity_id,
        f5::u8view apple_user_id) {
    fg::json user_values;
    fostlib::insert(user_values, "reference", reference);
    fostlib::insert(user_values, "identity_id", identity_id);
    fostlib::insert(user_values, "apple_user_id", apple_user_id);
    cnx.insert("odin.apple_credentials_ledger", user_values);
}


std::optional<f5::u8string> odin::apple::email_owner_identity_id(
        fostlib::pg::connection &cnx, fostlib::string email) {
    const f5::u8string sql(
            "SELECT oi.id FROM odin.identity oi INNER JOIN "
            "odin.apple_credentials fc ON oi.id = fc.identity_id WHERE "
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
