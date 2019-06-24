/**
    Copyright 2019 Felspar Co Ltd. <http://odin.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/

#include <odin/app.hpp>
#include <odin/credentials.hpp>
#include <odin/group.hpp>
#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>

#include <pqxx/except>
#include <fost/exception/parse_error.hpp>
#include <fost/insert>
#include <fost/json>
#include <fost/log>
#include <fostgres/sql.hpp>


namespace {

    fostlib::json parse_payload(fostlib::http::server::request &req) {
        // TODO: Support multiple ContentType
        auto body_str = fostlib::coerce<fostlib::string>(
                fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
        return fostlib::json::parse(body_str);
    }

    std::vector<f5::byte> load_key(
            fostlib::pg::connection &cnx,
            fostlib::json jwt_header,
            fostlib::json jwt_body) {
        auto const jwt_iss = fostlib::coerce<fostlib::string>(jwt_body["iss"]);
        if (jwt_iss.find(odin::c_app_namespace.value()) == std::string::npos) {
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "App namespace prefix does not match");
        }
        auto const app_id =
                jwt_iss.substr(odin::c_app_namespace.value().code_points());
        fostlib::json app = odin::app::get_detail(cnx, std::move(app_id));
        if (app.isnull()) {
            throw fostlib::exceptions::not_implemented(
                    __PRETTY_FUNCTION__, "App not found");
        }
        auto const app_token = odin::c_jwt_secret.value() + app_id;
        return std::vector<f5::byte>(
                app_token.data().begin(), app_token.data().end());
    }

    const class jwt_renewal : public fostlib::urlhandler::view {
      public:
        jwt_renewal() : view("odin.jwt.renewal") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {

            if (!req.headers().exists("__app")) {
                throw fostlib::exceptions::not_implemented(
                        __func__,
                        "The odin.jwt.renewal view must be wrapped by an "
                        "odin.app.secure "
                        "view on the secure path so that there is a valid JWT "
                        "to find App ID and User ID in");
            }

            if (req.method() != "GET")
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "Required GET, this should be a 405");

            auto const app_id = req.headers()["__app"].value();
            auto const jwt_user = req.headers()["__user"].value();
            
            // Check whether it is App's JWT or basic JWT
            if (app_id == jwt_user) {
                auto jwt = odin::app::mint_user_jwt(
                        jwt_user, app_id,
                        fostlib::coerce<fostlib::timediff>(config["expires"]));
                fostlib::mime::mime_headers headers;
                headers.add(
                        "Expires",
                        fostlib::coerce<fostlib::rfc1123_timestamp>(jwt.second)
                                .underlying()
                                .underlying());

                boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                        jwt.first, headers, L"application/jwt"));
                return std::make_pair(response, 200);
            } else {

                fostlib::pg::connection cnx{fostgres::connection(config, req)};
                static const fostlib::string sql("SELECT * FROM odin.identity WHERE odin.identity.id = $1");
                auto data = fostgres::sql(
                    cnx, sql, std::vector<fostlib::string>{jwt_user});
                auto &rs = data.second;
                auto row = rs.begin();
                if (row == rs.end()) {
                    throw fostlib::exceptions::not_implemented(
                        __func__,
                        "error -----------------------------1");
                    // return fostlib::json();
                }
                auto record = *row;

                if (++row != rs.end()) {
                    throw fostlib::exceptions::not_implemented(
                        __func__,
                        "error -----------------------------2");
                    // return fostlib::json();
                }
                fostlib::json user;
                for (std::size_t index{0}; index < record.size(); ++index) {
                    const auto parts = fostlib::split(data.first[index], "__");
                    if (parts.size() && parts[parts.size() - 1] == "tableoid") continue;
                    fostlib::jcursor pos;
                    for (const auto &p : parts) pos /= p;
                    fostlib::insert(user, pos, record[index]);
                }

                auto jwt_response(odin::mint_login_jwt(user));
                auto exp = jwt_response.expires(
                    fostlib::coerce<fostlib::timediff>(config["expires"]),
                    false);
                fostlib::mime::mime_headers headers;
                headers.add(
                    "Expires",
                    fostlib::coerce<fostlib::rfc1123_timestamp>(exp)
                            .underlying()
                            .underlying());

                boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    fostlib::utf8_string(jwt_response.token(
                            odin::c_jwt_secret.value().data())),
                    headers, L"application/jwt"));
                
                return std::make_pair(response, 200);
            }
        }

    } c_jwt_renewal;
}


const fostlib::urlhandler::view &odin::view::jwt_renewal = c_jwt_renewal;
