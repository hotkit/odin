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

    const class registration : public fostlib::urlhandler::view {
    public:
        registration()
        : view("odin.register") {
        }

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator () (
            const fostlib::json &config,
            const fostlib::string &path,
            fostlib::http::server::request &req,
            const fostlib::host &host
        ) const {
            if ( req.method() != "POST" ) {
                throw fostlib::exceptions::not_implemented(__func__,
                    "Registration requires POST. This should be a 405");
            }

            auto body_str = fostlib::coerce<fostlib::string>(
                fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            fostlib::json body = fostlib::json::parse(body_str);
            const auto username = fostlib::coerce<fostlib::nullable<f5::u8view>>(body["username"]);

            if ( not username || username.value().empty() ) {
                throw fostlib::exceptions::not_implemented("odin.register",
                    "Must pass username field");
            }

            fostlib::pg::connection cnx{fostgres::connection(config, req)};

            if (odin::does_user_exist(cnx, username.value())) {
                throw fostlib::exceptions::not_implemented("odin.register",
                    "User already exists");
            }

            const auto ref = odin::reference();
            odin::create_user(cnx, ref, username.value());

            if ( body.has_key("password") ) {
                const auto password = fostlib::coerce<f5::u8view>(body["password"]);
                if ( password.empty() ){
                    throw fostlib::exceptions::not_implemented("odin.register",
                        "Password cannot be empty");
                }
                odin::set_password(cnx, ref, username.value(), password);
            }

            if ( body.has_key("full_name") ) {
                const auto full_name = fostlib::coerce<f5::u8view>(body["full_name"]);
                if ( full_name.empty() ){
                    throw fostlib::exceptions::not_implemented("odin.register",
                        "Full name cannot be empty");
                }
                odin::set_full_name(cnx, ref, username.value(), full_name);
            }

            if ( body.has_key("email") ) {
                try {
                    const auto email = fostlib::coerce<fostlib::email_address>(body["email"]);
                    odin::set_email(cnx, ref, username.value(), email);
                } catch ( fostlib::exceptions::parse_error &e ) {
                    fostlib::log::debug(c_odin_registration)
                        ("", "Email parsing error")
                        ("username", username.value())
                        ("e-mail", body["email"]);
                    throw fostlib::exceptions::not_implemented("odin.register",
                        "Invalid e-mail address");
                }
            }

            cnx.commit();
            fostlib::mime::mime_headers headers;
            boost::shared_ptr<fostlib::mime> response(
                new fostlib::text_body(fostlib::json::unparse(fostlib::json{}, true),
                    headers, "application/json"));
            return std::make_pair(response, 201);
        }
    } c_registration;


}


const fostlib::urlhandler::view &odin::view::registration = c_registration;

