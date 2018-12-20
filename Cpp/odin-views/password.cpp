/**
    Copyright 2016-2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include <odin/credentials.hpp>
#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/pwhashproc.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <fost/insert>
#include <fost/log>
#include <fostgres/sql.hpp>
#include <fostgres/response.hpp>


namespace {

    const fostlib::module
            c_odin_reset_forgotten_password(odin::c_odin, "password.cpp");

    std::pair<boost::shared_ptr<fostlib::mime>, int>
            respond(fostlib::string message, int code = 403) {
        fostlib::json ret;
        if (not message.empty())
            fostlib::insert(ret, "message", std::move(message));
        fostlib::mime::mime_headers headers;
        boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                fostlib::json::unparse(ret, true), headers, "application/json"));
        return std::make_pair(response, code);
    }


    fostlib::json get_jwt_payload(fostlib::string t) {
        const auto parts = fostlib::split(t, ".");
        if (parts.size() != 3u) {
            throw fostlib::exceptions::not_implemented(
                    __func__, "Can't parse to jwt. This should be a 403");
        }
        const fostlib::base64_string b64_payload(parts[1].c_str());
        const auto v64_payload =
                fostlib::coerce<std::vector<unsigned char>>(b64_payload);
        const auto u8_payload =
                fostlib::coerce<fostlib::utf8_string>(v64_payload);
        const auto str_payload = fostlib::coerce<fostlib::string>(u8_payload);
        const auto unsafe_payload = fostlib::json::parse(str_payload);
        return unsafe_payload;
    }


    fostlib::json fetch_user_row(
            fostlib::pg::connection &cnx, const fostlib::string &identity_id) {
        fostlib::string query =
                "SELECT * FROM odin.credentials WHERE identity_id=$1";
        auto data = fostgres::sql(
                cnx, query, std::vector<fostlib::string>{identity_id});
        auto &rs = data.second;
        auto row = rs.begin();
        if (row == rs.end()) {
            fostlib::log::warning(c_odin_reset_forgotten_password)(
                    "", "Row not found");
            return fostlib::json();
        }
        auto record = *row;
        if (++row != rs.end()) {
            fostlib::log::error(c_odin_reset_forgotten_password)(
                    "", "More than one row returned");
            return fostlib::json();
        }

        fostlib::json result;
        for (std::size_t index{0}; index < record.size(); ++index) {
            const auto parts = fostlib::split(data.first[index], "__");
            if (parts.size() && parts[parts.size() - 1] == "tableoid") continue;
            fostlib::jcursor pos;
            for (const auto &p : parts) pos /= p;
            fostlib::insert(result, pos, record[index]);
        }

        return result;
    }


    const class password_me : public fostlib::urlhandler::view {
      public:
        password_me() : view("odin.password.me") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            auto body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            fostlib::json body = fostlib::json::parse(body_str);
            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            const auto reference = req.headers()["__odin_reference"].value();
            if (req.headers().exists("__user")) {
                const auto username = req.headers()["__user"].value();
                if (!body.has_key("new-password")
                    || !body.has_key("old-password")) {
                    return respond("Must supply both old and new password");
                }
                const auto old_password =
                        fostlib::coerce<f5::u8view>(body["old-password"]);
                const auto new_password =
                        fostlib::coerce<f5::u8view>(body["new-password"]);
                auto user = odin::credentials(
                        cnx, username, old_password, req.remote_address());
                cnx.commit();
                if (user.isnull()) return respond("Wrong password");
                if (new_password.bytes() < 8u)
                    return respond("New password is too short");
                odin::set_password(cnx, reference, username, new_password);
                auto logout_claim = req.headers()["__jwt"].subvalue(
                        odin::c_jwt_logout_claim.value());
                if (logout_claim)
                    odin::logout_user(
                            cnx, reference,
                            req.headers()["__remote_addr"].value(), username);
                cnx.commit();
                return respond("", 200);
            }
            return respond("No user is logged in", 401);
        }
    } c_password_me;


    const class forgotten_password_reset : public fostlib::urlhandler::view {
      public:
        forgotten_password_reset() : view("odin.password.reset-forgotten") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            if (req.method() != "POST") {
                throw fostlib::exceptions::not_implemented(
                        __func__,
                        "Forgotten password reset requires POST. This should "
                        "be a 405");
            }
            auto body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            fostlib::json body = fostlib::json::parse(body_str);
            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            if (!body.has_key("reset-password-token")
                || !body.has_key("new-password")) {
                return respond("Must supply both reset token and new password");
            }
            const auto reset_token = fostlib::coerce<fostlib::string>(
                    body["reset-password-token"]);
            fostlib::json json_payload = get_jwt_payload(reset_token);
            fostlib::string identity_id =
                    fostlib::coerce<f5::u8view>(json_payload["sub"]);
            fostlib::json user = fetch_user_row(cnx, identity_id);
            if (user.isnull()) { return respond("Not found", 404); }
            auto jwt = fostlib::jwt::token::load(
                    odin::c_jwt_reset_forgotten_password_secret.value()
                            + fostlib::coerce<fostlib::string>(
                                    user["password"]["hash"]),
                    reset_token);
            if (not jwt) { return respond("Invalid token", 403); }
            auto username =
                    fostlib::coerce<f5::u8view>(jwt.value().payload["sub"]);
            if (odin::does_user_exist(cnx, username)) {
                const auto reference = odin::reference();
                const auto new_password =
                        fostlib::coerce<f5::u8view>(body["new-password"]);
                odin::set_password(cnx, reference, username, new_password);
                if (odin::is_module_enabled(cnx, "opts/logout"))
                    odin::logout_user(
                            cnx, reference,
                            req.headers()["__remote_addr"].value(), username);
                cnx.commit();
                return respond("Success", 200);
            }
            throw fostlib::exceptions::not_implemented(
                    __func__, "Invalid reset-password-token.");
        }
    } c_reset_password;

    const class password_hash : public fostlib::urlhandler::view {
      public:
        password_hash() : view("odin.password.hash") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            if (!config.has_key("hash") || !config.has_key("verify")
                || !config.has_key("then")) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "Must supply 'hash', 'verify' and 'then' in the "
                        "configuration");
            }

            auto body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            fostlib::json body = fostlib::json::parse(body_str);
            auto hash_value = fostgres::datum(
                    config["hash"], {}, body, req);
            auto verify = fostgres::datum(
                    config["verify"], {}, body,
                    req);
            if (not hash_value || not verify || hash_value != verify) {
                return respond("Hashing failed", 422);
            }
            auto const hash_result = odin::hash_password(
                    fostlib::coerce<fostlib::string>(hash_value));
            req.headers().set("__hash", hash_result.first);
            req.headers().set(
                    "__hash_process",
                    fostlib::json::unparse(hash_result.second, true));
            return execute(config["then"], path, req, host);
        }
    } c_password_hash;


}

const fostlib::urlhandler::view &odin::view::password_hash = c_password_hash;
