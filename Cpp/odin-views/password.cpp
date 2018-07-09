/**
    Copyright 2016-2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include <odin/credentials.hpp>
#include <odin/nonce.hpp>
#include <odin/pwhashproc.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <fost/insert>
#include <fostgres/sql.hpp>


namespace {


    std::pair<boost::shared_ptr<fostlib::mime>, int> respond(
        fostlib::string message, int code=403
    ) {
        fostlib::json ret;
        if ( not message.empty() ) fostlib::insert(ret, "message", std::move(message));
        fostlib::mime::mime_headers headers;
        boost::shared_ptr<fostlib::mime> response(
            new fostlib::text_body(fostlib::json::unparse(ret, true),
                headers, "application/json"));
        return std::make_pair(response, code);
    }

    std::pair<boost::shared_ptr<fostlib::mime>, int> change_password(
        fostlib::pg::connection &cnx,
        fostlib::host remote_address,
        f5::u8view reference,
        f5::u8view username,
        f5::u8view old_password,
        f5::u8view new_password
    ) {
        if ( new_password.bytes() < 8u ) {
            return respond("New password is too short");
        }
        auto user = odin::credentials(cnx, username, old_password, remote_address);
        cnx.commit();
        if ( user.isnull() ) {
            return respond("Wrong password");
        }
        odin::set_password(cnx, reference, username, new_password);
        cnx.commit();
        return respond("", 200);
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
                const auto old_password = fostlib::coerce<f5::u8view>(body["old-password"]);
                const auto new_password = fostlib::coerce<f5::u8view>(body["new-password"]);
                fostlib::pg::connection cnx{fostgres::connection(config, req)};
                const auto reference = req.headers()["__odin_reference"].value();
                const auto remote_address = req.remote_address();
                return change_password(cnx, remote_address, reference, username, old_password, new_password);
            } else {
                return respond("No user is logged in");
            }
        }
    } c_password_me;


}


// const fostlib::urlhandler::view &odin::view::secure = c_secure;

