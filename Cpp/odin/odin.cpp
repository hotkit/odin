/**
    Copyright 2016-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */

#include <odin/odin.hpp>
#include <odin/nonce.hpp>
#include <odin/pwhashproc.hpp>

#include <fostgres/callback.hpp>
#include <fostgres/sql.hpp>


const fostlib::module odin::c_odin("odin");


const fostlib::setting<fostlib::string> odin::c_jwt_secret(
        "odin/odin.cpp", "odin", "JWT secret", odin::nonce(), true);

const fostlib::setting<bool>
        odin::c_jwt_trust("odin/odin.cpp", "odin", "Trust JWT", false, true);

const fostlib::setting<fostlib::string> odin::c_jwt_logout_claim(
        "odin/odin.cpp",
        "odin",
        "JWT logout claim",
        "http://odin.felspar.com/lo",
        true);

const fostlib::setting<bool> odin::c_jwt_logout_check(
        "odin/odin.cpp", "odin", "Perform JWT logout check", true, true);

const fostlib::setting<fostlib::string> odin::c_jwt_permissions_claim(
        "odin/odin.cpp",
        "odin",
        "JWT permissions claim",
        "http://odin.felspar.com/p",
        true);

const fostlib::setting<int64_t> odin::c_hash_rounds(
        "odin/odin.cpp", "odin", "Password hash rounds", 300000, true);

const fostlib::setting<fostlib::string>
        odin::c_jwt_reset_forgotten_password_secret(
                "odin/odin.cpp",
                "odin",
                "JWT reset forgotten password secret",
                odin::nonce(),
                true);

const fostlib::setting<fostlib::json> odin::c_facebook_apps(
        "odin/odin.cpp", "odin", "Facebook", fostlib::json{}, true);

const fostlib::setting<fostlib::string> odin::c_facebook_endpoint(
        "odin/odin.cpp",
        "odin",
        "Facebook API Endpoint",
        "https://graph.facebook.com",
        true);

const fostlib::setting<fostlib::json> odin::c_google_aud(
        "odin/odin.cpp", "odin", "Google", fostlib::null, true);

const fostlib::setting<fostlib::string> odin::c_app_namespace(
        "odin/odin.cpp",
        "odin",
        "Application namespace",
        "http://odin.felspar.com/app/",
        true);


bool odin::is_module_enabled(
        fostlib::pg::connection &cnx, f5::u8view module_name) {
    static const fostlib::string sql("SELECT * FROM odin.module WHERE name=$1");
    auto data =
            fostgres::sql(cnx, sql, std::vector<fostlib::string>{module_name});
    auto &rs = data.second;
    return rs.begin() != rs.end();
}


namespace {
    const fostgres::register_cnx_callback
            c_cb([](fostlib::pg::connection &cnx,
                    const fostlib::http::server::request &req) {
                if (req.headers().exists("__user")) {
                    const auto &user = req.headers()["__user"];
                    cnx.set_session("odin.jwt.sub", user.value());
                }
                if (req.headers().exists("__app")) {
                    const auto &app = req.headers()["__app"];
                    cnx.set_session("odin.jwt.app", app.value());
                }
                if (req.headers().exists("__jwt")) {
                    for (const auto &sv : req.headers()["__jwt"])
                        cnx.set_session("odin.jwt." + sv.first, sv.second);
                }
                if (req.headers().exists("__odin_reference")) {
                    const auto &reference = req.headers()["__odin_reference"];
                    cnx.set_session("odin.reference", reference.value());
                }
            });
}
