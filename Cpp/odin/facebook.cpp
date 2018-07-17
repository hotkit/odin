/**
    Copyright 2018 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/

#include <odin/facebook.hpp>
#include <odin/fg/native.hpp>
#include <odin/odin.hpp>

#include <fost/http>
#include <fost/log>
#include <fostgres/sql.hpp>

#include <fost/insert>

namespace odin {

    namespace facebook {

        fostlib::string get_app_token() {
            fostlib::url base_url(odin::c_facebook_endpoint.value());
            fostlib::url::filepath_string api{"/oauth/access_token"};
            fostlib::url::query_string qs{};
            fostlib::url fb_url(base_url, api);
            qs.append("client_id", odin::c_facebook_app_id.value());
            qs.append("client_secret", odin::c_facebook_secret.value());
            qs.append("grant_type", "client_credentials");
            fb_url.query(qs);
            fostlib::http::user_agent ua(fb_url);
            auto response = ua.get(fb_url);
            auto response_data = fostlib::coerce<fostlib::string>(fostlib::coerce<fostlib::utf8_string>(response->body()->data()));
            fostlib::json body = fostlib::json::parse(response_data);
            return fostlib::coerce<fostlib::string>(body["access_token"]);
        }


        bool is_user_authenticated(f5::u8view app_token, f5::u8view user_token) {
            fostlib::url base_url(odin::c_facebook_endpoint.value());
            fostlib::url::filepath_string api{"/debug_token"};
            fostlib::url fb_url(base_url, api);
            fostlib::url::query_string qs{};
            qs.append("input_token", user_token);
            qs.append("access_token", app_token);
            fb_url.query(qs);
            fostlib::http::user_agent ua(fb_url);
            auto response = ua.get(fb_url);
            auto response_data = fostlib::coerce<fostlib::string>(fostlib::coerce<fostlib::utf8_string>(response->body()->data()));
            fostlib::json body = fostlib::json::parse(response_data);
            return fostlib::coerce<bool>(body["data"]["is_valid"]);
        }


        fostlib::json get_user_detail(f5::u8view user_token) {
            const auto app_token = get_app_token();
            if ( !is_user_authenticated(app_token, user_token) ) {
                return fostlib::json{};
            }
            fostlib::url base_url(odin::c_facebook_endpoint.value());
            fostlib::url::filepath_string api{"/me"};
            fostlib::url fb_url(base_url, api);
            fostlib::url::query_string qs{};
            qs.append("access_token", user_token);
            qs.append("fields", "id,name,email");
            fb_url.query(qs);
            fostlib::http::user_agent ua(fb_url);
            auto response = ua.get(fb_url);
            auto response_data = fostlib::coerce<fostlib::string>(fostlib::coerce<fostlib::utf8_string>(response->body()->data()));
            return fostlib::json::parse(response_data);
        }


        fostlib::json credentials(fostlib::pg::connection &cnx, const f5::u8view &user_id) {
            static const fostlib::string sql("SELECT "
                    "odin.identity.tableoid AS identity__tableoid, "
                    "odin.facebook_credentials.tableoid AS facebook_credentials__tableoid, "
                    "odin.identity.*, odin.facebook_credentials.* "
                "FROM odin.facebook_credentials "
                "JOIN odin.identity ON odin.identity.id=odin.facebook_credentials.identity_id "
                "WHERE odin.facebook_credentials.facebook_user_id = $1");
            auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{user_id});
            auto &rs = data.second;
            auto row = rs.begin();
            if ( row == rs.end() ) {
                fostlib::log::warning(c_odin)
                    ("", "Facebook user not found")
                    ("facebook_user_id", user_id);
                return fostlib::json();
            }
            auto record = *row;
            if ( ++row != rs.end() ) {
                fostlib::log::error(c_odin)
                    ("", "More than one facebook user returned")
                    ("facebook_user_id", user_id);
                return fostlib::json();
            }

            fostlib::json user;
            for ( std::size_t index{0}; index < record.size(); ++index ) {
                const auto parts = fostlib::split(data.first[index], "__");
                if ( parts.size() && parts[parts.size() - 1] == "tableoid" )
                    continue;
                fostlib::jcursor pos;
                for ( const auto &p : parts ) pos /= p;
                fostlib::insert(user, pos, record[index]);
            }
            return user;
        }


        void set_facebook_credentials(
            fostlib::pg::connection &cnx, f5::u8view reference, f5::u8view identity_id, f5::u8view facebook_user_id
        ) {
            fg::json user_values;
            fostlib::insert(user_values, "reference", reference);
            fostlib::insert(user_values, "identity_id", identity_id);
            fostlib::insert(user_values, "facebook_user_id", facebook_user_id);
            cnx.insert("odin.facebook_credentials_ledger", user_values);
        }



    }

}
