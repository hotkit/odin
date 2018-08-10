/**
    Copyright 2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/

#include <odin/app.hpp>
#include <odin/credentials.hpp>
#include <odin/odin.hpp>
#include <odin/nonce.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <fost/exception/parse_error.hpp>
#include <fost/insert>
#include <fost/log>
#include <fost/mailbox>
#include <fostgres/sql.hpp>


namespace {

    const fostlib::module c_odin_app(odin::c_odin, "app.cpp");

    fostlib::json parse_payload(fostlib::http::server::request &req) {
        // TODO: Support multiple ContentType
        auto body_str = fostlib::coerce<fostlib::string>(
            fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
        return fostlib::json::parse(body_str);
    }

    const class app_login : public fostlib::urlhandler::view {
    public:
        app_login()
        : view("odin.app.login") {
        }

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator () (
            const fostlib::json &config,
            const fostlib::string &path,
            fostlib::http::server::request &req,
            const fostlib::host &host
        ) const {
            if ( req.method() == "GET"){
                fostlib::mime::mime_headers headers;
                boost::shared_ptr<fostlib::mime> response(
                    new fostlib::text_body(fostlib::utf8_string("Login Page"), headers, L"text/html"));
                return std::make_pair(response, 200);
            }

            if ( req.method() != "POST" )
                throw fostlib::exceptions::not_implemented(__func__,
                    "App Login required POST, this should be a 405");
            fostlib::json body = parse_payload(req);
            if ( !body.has_key("username") || !body.has_key("password") || !body.has_key("app_id") )
                throw fostlib::exceptions::not_implemented(__func__,
                    "Must pass both username, password and app_id fields");

            const auto username = fostlib::coerce<fostlib::string>(body["username"]);
            const auto password = fostlib::coerce<fostlib::string>(body["password"]);
            const auto app_id = fostlib::coerce<fostlib::string>(body["app_id"]);

            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            auto user = odin::credentials(cnx, username, password, req.remote_address());
            cnx.commit();
            if ( user.isnull() )
                throw fostlib::exceptions::not_implemented(__func__, "User not found");
            fostlib::json app = odin::app::get_detail(cnx, app_id);
            if ( app.isnull() )
                throw fostlib::exceptions::not_implemented(__func__, "App not found");

            auto jwt = odin::app::mint_user_jwt(user, app);

            fostlib::mime::mime_headers headers;
            auto exp = jwt.expires(fostlib::coerce<fostlib::timediff>(config["expires"]), false);
            headers.add("Expires", fostlib::coerce<fostlib::rfc1123_timestamp>(exp).underlying().underlying().c_str());
            const auto jwt_token = fostlib::utf8_string(jwt.token());
            const auto redirect_url = fostlib::coerce<fostlib::string>(app["app"]["redirect_url"]);

            // Do we have a better way to doing this?
            headers.add("Location", redirect_url + "#" + jwt_token);
            boost::shared_ptr<fostlib::mime> response(
                    new fostlib::text_body(jwt_token, headers, L"application/jwt"));
            return std::make_pair(response, 303);
        }

    } c_app_login;


}


const fostlib::urlhandler::view &odin::view::app_login = c_app_login;

