/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/credentials.hpp>
#include <odin/views.hpp>


namespace {


    const class login : public fostlib::urlhandler::view {
        public:
            login()
            : view("odin.login") {
            }

            std::pair<boost::shared_ptr<fostlib::mime>, int> operator () (
                const fostlib::json &config, const fostlib::string &,
                fostlib::http::server::request &req,
                const fostlib::host &
            ) const {
                auto body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
                fostlib::json body = fostlib::json::parse(body_str);
                if ( !body.has_key("username") || !body.has_key("password") ) {
                    throw fostlib::exceptions::not_implemented("odin.login",
                        "Must pass both username and password fields");
                }
                const auto username = fostlib::coerce<fostlib::string>(body["username"]);
                const auto password = fostlib::coerce<fostlib::string>(body["password"]);
                if ( username.empty() || password.empty() ) {
                    throw fostlib::exceptions::not_implemented("odin.login",
                        "Must pass both a username and password");
                }
                fostlib::pg::connection cnx{config};
                auto user = odin::credentials(cnx, username, password);
                cnx.commit();
                if ( user.isnull() ) {
                    throw fostlib::exceptions::not_implemented("Not authenticated");
                } else {
                    const bool pretty = fostlib::coerce<fostlib::nullable<bool>>(config["pretty"]).value(true);
                    boost::shared_ptr<fostlib::mime> response(
                            new fostlib::text_body(fostlib::json::unparse(user, pretty),
                                fostlib::mime::mime_headers(), L"application/json"));
                    return std::make_pair(response, 200);
                }
            }
    } c_login;


}


const fostlib::urlhandler::view &odin::view::login = c_login;

