/**
    Copyright 2016-2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/

#include <odin/credentials.hpp>
#include <odin/google.hpp>
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

    const fostlib::module c_odin_google(odin::c_odin, "google.cpp");

    const class google : public fostlib::urlhandler::view {
    public:
        google()
        : view("odin.google.login") {
        }

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator () (
            const fostlib::json &config,
            const fostlib::string &path,
            fostlib::http::server::request &req,
            const fostlib::host &host
        ) const {
            if ( req.method() != "POST" )
                throw fostlib::exceptions::not_implemented(__func__,
                    "Registration requires POST. This should be a 405");
            auto body_str = fostlib::coerce<fostlib::string>(
                fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            fostlib::json body = fostlib::json::parse(body_str);
            if ( !body.has_key("access_token") )
                throw fostlib::exceptions::not_implemented("odin.google.login",
                    "Must pass access_token field");
            const auto access_token = fostlib::coerce<fostlib::string>(body["access_token"]);
            if ( !odin::google::is_user_authenticated(access_token) )
                throw fostlib::exceptions::not_implemented("odin.google.login",
                    "User not authenticated");

            auto user_detail = odin::google::get_user_detail(access_token);
            const auto google_user_id = fostlib::coerce<f5::u8view>(user_detail["sub"]);
            const auto google_user_name = fostlib::coerce<f5::u8view>(user_detail["name"]);
            const auto google_user_email = fostlib::coerce<fostlib::email_address>(user_detail["email"]);

            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            const auto reference = odin::reference();

            auto google_user = odin::google::credentials(cnx, google_user_id);
            auto identity_id = reference;
            if ( google_user.isnull() ) {
                odin::create_user(cnx, reference, identity_id);
                odin::set_full_name(cnx, reference, identity_id, google_user_name);
                odin::set_email(cnx, reference, identity_id, google_user_email);
            } else {
                const fostlib::jcursor id("identity", "id");
                identity_id = fostlib::coerce<fostlib::string>(google_user[id]);
            }
            odin::google::set_google_credential(cnx, reference, identity_id, google_user_id);
            cnx.commit();

            google_user = odin::google::credentials(cnx, google_user_id);
            fostlib::log::warning(c_odin_google)("google_user", google_user)("config", config);

            auto jwt(odin::mint_login_jwt(google_user));
            auto exp = jwt.expires(fostlib::coerce<fostlib::timediff>(config["expires"]), false);
            jwt.claim("google_user_id", google_user["google_credentials"]["google_user_id"]);
            fostlib::mime::mime_headers headers;
            headers.add("Expires", fostlib::coerce<fostlib::rfc1123_timestamp>(exp).underlying().underlying().c_str());
            boost::shared_ptr<fostlib::mime> response(
                    new fostlib::text_body(fostlib::utf8_string(jwt.token()),
                        headers, L"application/jwt"));
            return std::make_pair(response, 200);
        }
    } c_google;


}


const fostlib::urlhandler::view &odin::view::google = c_google;

