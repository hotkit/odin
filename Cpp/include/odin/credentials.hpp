/**
    Copyright 2018-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <fost/crypto>
#include <fost/internet>
#include <fost/postgres>


namespace odin {


    /// Return the database row for the identity together with the
    /// credentials, but only if the supplied password is correct. If the
    /// credentials failed to match an empty JSON instance (null) is
    /// returned.
    fostlib::json credentials(
            fostlib::pg::connection &cnx,
            const fostlib::string &username,
            const fostlib::string &password,
            const fostlib::nullable<fostlib::host> &source = fostlib::null,
            const fostlib::json &annotation = fostlib::json::object_t());

    /// Return the database row for the identity together with the
    /// credentials and app_user, but only if the supplied password is correct.
    /// If the credentials failed to match an empty JSON instance (null) is
    /// returned.
    fostlib::json app_credentials(
            fostlib::pg::connection &cnx,
            const fostlib::string &username,
            const fostlib::string &password,
            const fostlib::string &app_id,
            const fostlib::nullable<fostlib::host> &source = fostlib::null,
            const fostlib::json &annotation = fostlib::json::object_t());

    /// Mint Login JWT for this user and set common fields on it
    fostlib::jwt::mint mint_login_jwt(
            const fostlib::json &user, fostlib::json payload = fostlib::json{});

    /// Mint Reset Password JWT for this user and set common fields on it
    fostlib::jwt::mint mint_reset_password_jwt(const f5::u8view username);

    /// Mint new JWT token based on the old one
    std::pair<fostlib::utf8_string, fostlib::timestamp> renew_jwt(
            fostlib::string jwt,
            fostlib::string secret,
            const fostlib::json config);
}
