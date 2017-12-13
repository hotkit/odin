/*
    Copyright 2016-2017 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/credentials.hpp>
#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/views.hpp>

#include <fost/insert>
#include <fost/log>
#include <fostgres/sql.hpp>


namespace {


    const fostlib::module c_odin_login(odin::c_odin, "login.cpp");


    const class login : public fostlib::urlhandler::view {
    public:
        login()
        : view("odin.login") {
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
                fostlib::pg::connection cnx{fostgres::connection(config, req)};
                odin::reference(cnx);
                auto user = odin::credentials(cnx, username, password, req.remote_address());
                cnx.commit();
                if ( user.isnull() ) {
                    return execute(config["failure"], path, req, host);
                } else {
                    auto jwt(odin::mint_jwt(user));
                    auto exp = jwt.expires(fostlib::coerce<fostlib::timediff>(config["expires"]), false);

                    if ( config.has_key("permissions") && config["permissions"].isarray() ) {
                        fostlib::json allowed;
                        for ( const fostlib::json &perm : config["permissions"] ) {
                            fostlib::json where;
                            fostlib::insert(where, "identity_id", username);
                            fostlib::insert(where, "permission_slug", perm);
                            auto rs = cnx.select("odin.user_permission", where);
                            bool granted = rs.begin() != rs.end();
                            if ( granted ) {
                                fostlib::push_back(allowed, perm);
                            }
                            fostlib::log::debug(c_odin_login)
                                ("allowed", allowed)
                                ("granted", granted)
                                ("permission", perm)
                                ("where", where);
                        }
                        if ( not allowed.isnull() ) {
                            jwt.claim(odin::c_jwt_permissions_claim.value(), allowed);
                        }
                    }

                    fostlib::mime::mime_headers headers;
                    headers.add("Expiress", fostlib::coerce<fostlib::rfc1123_timestamp>(exp).underlying().underlying().c_str());
                    boost::shared_ptr<fostlib::mime> response(
                            new fostlib::text_body(fostlib::utf8_string(jwt.token()),
                                headers, L"application/jwt"));
                    return std::make_pair(response, 200);
                }
            } else {
                throw fostlib::exceptions::not_implemented(__func__,
                    "Login requires POST. This should be a 405");
            }
        }
    } c_login;


}


const fostlib::urlhandler::view &odin::view::login = c_login;

