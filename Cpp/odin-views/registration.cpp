/**
    Copyright 2016-2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/

#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <fost/exception/parse_error.hpp>
#include <fost/insert>
#include <fost/log>
#include <fost/mailbox>
#include <fostgres/sql.hpp>


namespace {

    const fostlib::module c_odin_registration(odin::c_odin, "registration.cpp");

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

    const class registration : public fostlib::urlhandler::view {
      public:
        registration() : view("odin.register") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            if (req.method() != "POST") {
                throw fostlib::exceptions::not_implemented(
                        __func__,
                        "Registration requires POST. This should be a 405");
            }

            auto body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            fostlib::json body = fostlib::json::parse(body_str);
            const auto username =
                    fostlib::coerce<fostlib::nullable<f5::u8view>>(
                            body["username"]);

            if (not username || username.value().empty()) {
                throw fostlib::exceptions::not_implemented(
                        "odin.register", "Must pass username field");
            }

            fostlib::pg::connection cnx{fostgres::connection(config, req)};

            if (odin::does_user_exist(cnx, username.value())) {
                throw fostlib::exceptions::not_implemented(
                        "odin.register", "User already exists");
            }

            const auto ref = odin::reference();
            // Use reference as identity_id
            const auto identity_id = ref;
            odin::create_user(cnx, identity_id);

            if (body.has_key("password")) {
                const auto password =
                        fostlib::coerce<f5::u8view>(body["password"]);
                if (password.empty() || password.bytes() < 8u) {
                    throw fostlib::exceptions::not_implemented(
                            "odin.register", "Invalid password");
                }
                odin::set_password(
                        cnx, ref, identity_id, username.value(), password);
            }

            if (body.has_key("full_name")) {
                const auto full_name =
                        fostlib::coerce<f5::u8view>(body["full_name"]);
                if (full_name.empty()) {
                    throw fostlib::exceptions::not_implemented(
                            "odin.register", "Full name cannot be empty");
                }
                odin::set_full_name(cnx, ref, identity_id, full_name);
            }

            if (body.has_key("phone_number")) {
                const auto phone_number =
                        fostlib::coerce<f5::u8view>(body["phone_number"]);
                odin::set_phone_number(cnx, ref, identity_id, phone_number);
            }

            if (body.has_key("email")) {
                try {
                    const auto email = fostlib::coerce<fostlib::email_address>(
                            body["email"]);
                    if (odin::does_email_exist(
                                cnx,
                                fostlib::coerce<fostlib::string>(
                                        body["email"]))) {
                        return respond("This email already exists", 422);
                    }
                    odin::set_email(cnx, ref, identity_id, email);
                } catch (fostlib::exceptions::parse_error &e) {
                    fostlib::log::error(c_odin_registration)(
                            "", "Email parsing error")(
                            "username",
                            username.value())("e-mail", body["email"]);
                    throw fostlib::exceptions::not_implemented(
                            "odin.register", "Invalid e-mail address");
                } catch (std::exception &e) {
                    fostlib::log::error(c_odin_registration)(
                            "", "Email already exists")(
                            "username",
                            username.value())("e-mail", body["email"]);
                    throw fostlib::exceptions::not_implemented(
                            "odin.register", e.what());
                }
            }

            cnx.commit();
            fostlib::mime::mime_headers headers;
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    fostlib::json::unparse(fostlib::json{}, true), headers,
                    "application/json"));
            return std::make_pair(response, 201);
        }
    } c_registration;


}


const fostlib::urlhandler::view &odin::view::registration = c_registration;
