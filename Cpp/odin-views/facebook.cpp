/**
    Copyright 2016-2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/

#include <odin/credentials.hpp>
#include <odin/facebook.hpp>
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

    const fostlib::module c_odin_facebook(odin::c_odin, "facebook.cpp");

    const class facebook : public fostlib::urlhandler::view {
    public:
        facebook()
        : view("odin.facebook.login") {
        }

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator () (
            const fostlib::json &config,
            const fostlib::string &path,
            fostlib::http::server::request &req,
            const fostlib::host &host
        ) const {
            if ( req.method() != "POST" )
                throw fostlib::exceptions::not_implemented(__func__,
                    "Facebook login requires POST. This should be a 405");

            auto body_str = fostlib::coerce<fostlib::string>(
                fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            fostlib::json body = fostlib::json::parse(body_str);
            if ( !body.has_key("access_token") )
                throw fostlib::exceptions::not_implemented("odin.facebook.login",
                    "Must pass access_token field");
            const auto access_token = fostlib::coerce<fostlib::string>(body["access_token"]);
            fostlib::json user_detail;
            if ( config.has_key("facebook-mock") ) {
                if ( fostlib::coerce<fostlib::string>(config["facebook-mock"]) == "OK") {
                    // Use access token as facebook ID
                    fostlib::insert(user_detail, "id", access_token);
                    fostlib::insert(user_detail, "name", "Test User");
                    fostlib::insert(user_detail, "email", access_token + "@example.com");
                }
            } else {
                user_detail = odin::facebook::get_user_detail(access_token);
            }

            if ( user_detail.isnull() )
                throw fostlib::exceptions::not_implemented("odin.facebook.login",
                    "User not authenticated");
            const auto facebook_user_id = fostlib::coerce<f5::u8view>(user_detail["id"]);
            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            const auto reference = odin::reference();
            auto facebook_user = odin::facebook::credentials(cnx, facebook_user_id);
            auto identity_id = reference;
            if ( facebook_user.isnull() ) {
                odin::create_user(cnx, reference, identity_id);
                if ( user_detail.has_key("name") ) {
                    const auto facebook_user_name = fostlib::coerce<f5::u8view>(user_detail["name"]);
                    odin::set_full_name(cnx, reference, identity_id, facebook_user_name);
                }
                if ( user_detail.has_key("email") ) {
                    const auto facebook_user_email = fostlib::coerce<fostlib::email_address>(user_detail["email"]);
                    odin::set_email(cnx, reference, identity_id, facebook_user_email);
                }
            } else {
                const fostlib::jcursor id("identity", "id");
                identity_id = fostlib::coerce<fostlib::string>(facebook_user[id]);
            }
            odin::facebook::set_facebook_credentials(cnx, reference, identity_id, facebook_user_id);
            cnx.commit();

            facebook_user = odin::facebook::credentials(cnx, facebook_user_id);

            auto jwt(odin::mint_login_jwt(facebook_user));
            auto exp = jwt.expires(fostlib::coerce<fostlib::timediff>(config["expires"]), false);
            jwt.claim("facebook_user_id", facebook_user["facebook_credentials"]["facebook_user_id"]);
            fostlib::mime::mime_headers headers;
            headers.add("Expires", fostlib::coerce<fostlib::rfc1123_timestamp>(exp).underlying().underlying().c_str());
            boost::shared_ptr<fostlib::mime> response(
                    new fostlib::text_body(fostlib::utf8_string(jwt.token()),
                        headers, L"application/jwt"));
            return std::make_pair(response, 200);
        }
    } c_facebook;


}


const fostlib::urlhandler::view &odin::view::facebook = c_facebook;

