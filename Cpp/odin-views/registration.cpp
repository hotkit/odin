/**
    Copyright 2016-2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/

#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <fost/insert>
#include <fost/log>
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

            if ( !body.has_key("username") ) {
                throw fostlib::exceptions::not_implemented("odin.register",
                    "Must pass username field");
            }

            const auto username = fostlib::coerce<f5::u8view>(body["username"]);
            if ( username.empty() ) {
                throw fostlib::exceptions::not_implemented("odin.register",
                    "Must pass username");
            }

            fostlib::pg::connection cnx{fostgres::connection(config, req)};

            if (odin::does_user_exist(cnx, username)) {
                throw fostlib::exceptions::not_implemented("odin.register",
                    "User already exists");
            }

            const auto ref = odin::reference();
            odin::create_user(cnx, ref, username);

            if ( body.has_key("password") ) {
                const auto password = fostlib::coerce<f5::u8view>(body["password"]);
                if ( password.empty() ){
                    throw fostlib::exceptions::not_implemented("odin.register",
                        "Password cannot be empty");
                }
                odin::set_password(cnx, ref, username, password);
            }

            if ( body.has_key("full_name") ) {
                const auto full_name = fostlib::coerce<f5::u8view>(body["full_name"]);
                if ( full_name.empty() ){
                    throw fostlib::exceptions::not_implemented("odin.register",
                        "Full name cannot be empty");
                }
                odin::set_full_name(cnx, ref, username, full_name);
            }

            if ( body.has_key("email") ) {
                const auto email = fostlib::coerce<f5::u8view>(body["email"]);
                if ( email.empty() ){
                    throw fostlib::exceptions::not_implemented("odin.register",
                        "Full name cannot be empty");
                }
                odin::set_email(cnx, ref, username, email);
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

