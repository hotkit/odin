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
            if ( req.method() == "POST" ) {
                auto body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
                fostlib::json body = fostlib::json::parse(body_str);
                // Should retrieve confirm_password as well?
                if ( !body.has_key("username") || !body.has_key("password") ) {
                    throw fostlib::exceptions::not_implemented("odin.register",
                        "Must pass both username and password fields");
                }
                const auto username = fostlib::coerce<f5::u8view>(body["username"]);
                const auto password = fostlib::coerce<f5::u8view>(body["password"]);
                if ( username.empty() || password.empty() ) {
                    throw fostlib::exceptions::not_implemented("odin.register",
                        "Must pass both a username and password");
                }
                fostlib::pg::connection cnx{fostgres::connection(config, req)};

                if (odin::does_user_exist(cnx, username)) {
                    throw fostlib::exceptions::not_implemented("odin.register",
                        "User already exists");
                }

                const auto ref = odin::reference();
                odin::create_user(cnx, ref, username);
                odin::set_password(cnx, ref, username, password);
                cnx.commit();
                fostlib::mime::mime_headers headers;
                fostlib::json ret;
                fostlib::insert(ret, "message", "Success");
                boost::shared_ptr<fostlib::mime> response(
                    new fostlib::text_body(fostlib::json::unparse(ret, true),
                        headers, "application/json"));
                return std::make_pair(response, 201);
            } else {
                throw fostlib::exceptions::not_implemented(__func__,
                    "Registration requires POST. This should be a 405");
            }
        }
    } c_registration;


}


const fostlib::urlhandler::view &odin::view::registration = c_registration;

