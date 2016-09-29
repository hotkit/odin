/*
    Copyright 2016 Felspar Co Ltd. http://odin.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <odin/credentials.hpp>
#include <odin/nonce.hpp>
#include <odin/pwhashproc.hpp>
#include <odin/views.hpp>

#include <fost/insert>
#include <fostgres/sql.hpp>


namespace {


    std::pair<boost::shared_ptr<fostlib::mime>, int> respond(
        const fostlib::string &message, int code=403
    ) {
        fostlib::json ret;
        fostlib::insert(ret, "message", message);
        fostlib::mime::mime_headers headers;
        boost::shared_ptr<fostlib::mime> response(
            new fostlib::text_body(fostlib::json::unparse(ret, true),
                headers, "application/json"));
        return std::make_pair(response, code);
    }


    const class password_me : public fostlib::urlhandler::view {
    public:
        password_me()
        : view("odin.password.me") {
        }

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator () (
            const fostlib::json &config, const fostlib::string &path,
            fostlib::http::server::request &req,
            const fostlib::host &host
        ) const {
            if ( req.headers().exists("__user") ) {
                const auto username = req.headers()["__user"].value();
                auto body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
                fostlib::json body = fostlib::json::parse(body_str);
                if ( !body.has_key("new-password") || !body.has_key("old-password") ) {
                    return respond("Must supply both old and new password");
                }
                const auto old_password = fostlib::coerce<fostlib::string>(body["old-password"]);
                const auto new_password = fostlib::coerce<fostlib::string>(body["new-password"]);
                fostlib::pg::connection cnx{fostgres::connection(config, req)};
                odin::reference(cnx);
                auto user = odin::credentials(cnx, username, old_password, req.remote_address());
                cnx.commit();
                if ( user.isnull() ) {
                    return respond("Wrong password");
                } else {
                    if ( new_password.length() < 8u ) {
                        return respond("New password is too short");
                    } else {
                        const auto hash = odin::set_password(new_password);
                        fostlib::json row;
                        fostlib::insert(row, "reference", req.headers()["__odin_reference"].value());
                        fostlib::insert(row, "identity_id", username);
                        fostlib::insert(row, "password", hash.first);
                        fostlib::insert(row, "process", hash.second);
                        cnx.insert("odin.credentials_password_ledger", row);
                        cnx.commit();
                        return respond("Password changed", 200);
                    }
                }
            } else {
                return respond("No user is logged in");
            }
        }
    } c_password_me;


}


// const fostlib::urlhandler::view &odin::view::secure = c_secure;

