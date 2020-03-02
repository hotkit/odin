/**
    Copyright 2018-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <odin/app.hpp>
#include <odin/nonce.hpp>
#include <odin/odin.hpp>
#include <odin/views.hpp>

#include <fost/crypto>
#include <fost/insert>
#include <fost/log>
#include <fostgres/sql.hpp>


namespace {

    bool has_installation_id_been_claimed(
            fostlib::pg::connection &cnx,
            fostlib::string const app_id,
            fostlib::string const installation_id) {
        static const fostlib::string sql(
                "SELECT installation_id FROM "
                "odin.app_user_installation_id_ledger "
                "WHERE app_id=$1 AND installation_id=$2");
        auto data = fostgres::sql(
                cnx, sql,
                std::vector<fostlib::string>{app_id, installation_id});
        auto &rs = data.second;
        return rs.begin() != rs.end();
    }

    const class app_installation : public fostlib::urlhandler::view {
      public:
        app_installation() : view("odin.app.installation") {}

        static const fostlib::jcursor apploc;

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {

            if (req.method() != "POST") {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "App Login required POST, this should be a 405");
            }

            if (!req.headers().exists("__app")) {
                throw fostlib::exceptions::not_implemented(
                        __func__,
                        "The odin.app.installation view must be wrapped by an "
                        "odin.app.secure "
                        "view on the secure path so that there is a valid JWT "
                        "to find the App ID in");
            }

            auto const body_str = fostlib::coerce<fostlib::string>(
                    fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
            auto const body = fostlib::json::parse(body_str);

            if (!body.has_key("installation_id")) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__, "Must pass installation_id field");
            }

            auto const app_id = req.headers()["__app"].value();
            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            fostlib::json const app = odin::app::get_detail(cnx, app_id);

            // Not support INVITE_ONLY application
            if (app["app"]["access_policy"] == fostlib::json{"INVITE_ONLY"}) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__, "Forbidden");
            }

            auto const installation_id =
                    fostlib::coerce<fostlib::string>(body["installation_id"]);
            if (has_installation_id_been_claimed(cnx, app_id, installation_id)) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "The installation_id has been claimed.");
            }
            auto const reference = odin::reference();
            /// Using reference as an identity_id
            auto const identity_id = reference;
            auto const app_user_id = reference;
            odin::app::save_app_user(
                    cnx, reference, app_id, identity_id, app_user_id);
            odin::app::set_installation_id(
                    cnx, reference, app_id, identity_id, installation_id);
            cnx.commit();
            auto jwt = odin::app::mint_user_jwt(
                    app_user_id, app_id,
                    fostlib::coerce<fostlib::timediff>(config["expires"]));
            fostlib::mime::mime_headers headers;
            headers.add(
                    "Expires",
                    fostlib::coerce<fostlib::rfc1123_timestamp>(jwt.second)
                            .underlying()
                            .underlying());
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    jwt.first, headers, L"application/jwt"));

            return std::make_pair(response, 201);
        }
    } c_app_installation;

    const fostlib::jcursor app_installation::apploc("headers", "__app");
}


const fostlib::urlhandler::view &odin::view::app_installation =
        c_app_installation;
