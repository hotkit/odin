/**
    Copyright 2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */

#include <odin/app.hpp>
#include <odin/credentials.hpp>
#include <odin/odin.hpp>
#include <odin/nonce.hpp>
#include <odin/user.hpp>
#include <odin/views.hpp>
#include <fost/insert>
#include <fost/json>
#include <fost/log>
#include <fost/push_back>
#include <fostgres/sql.hpp>

namespace {

    fostlib::json get_app_user(
            fostlib::pg::connection &cnx, const f5::u8view app_id, const f5::u8view app_user_id) {
        const f5::u8view sql{
                "SELECT "
                "* "
                "FROM odin.app_user "
                "WHERE odin.app_user.app_id = $1 AND odin.app_user.app_user_id = $2; "};

        auto data = fostgres::sql(cnx, sql, std::vector<fostlib::string>{app_id, app_user_id});
        auto &rs = data.second;
        auto row = rs.begin();
        if (row == rs.end()) {
            fostlib::log::warning(odin::c_odin)("", "App not found")("app_id", app_id)("app_user_id", app_user_id);
            return fostlib::json();
        }
        auto record = *row;
        fostlib::json app;
        for (std::size_t index{0}; index < record.size(); ++index) {
            fostlib::insert(app, data.first[index], record[index]);
        }
        return app;
    }

    const class app_user : public fostlib::urlhandler::view {
      public:
        app_user() : view("odin.app.user") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {

            if (not req.headers().exists("__app")) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "The odin.app_user view must be wrapped by an "
                        "odin.app.secure"
                        "view on the secure path so that there is a valid JWT "
                        "to find App ID");
            }


            if (req.method() != "PUT") {
                fostlib::json config;
                fostlib::insert(config, "view", "fost.response.405");
                fostlib::push_back(config, "configuration", "allow", "PUT");
                return execute(config, path, req, host);
            }

            auto parameters = fostlib::split(path, '/');
            if (parameters.size() != 2) {
                fostlib::json config;
                fostlib::insert(config, "view", "fost.response.404");
                return execute(config, path, req, host);
            }
            auto const app_id = parameters[0];
            auto const app_user_id = parameters[1];
            auto const app_data = fostlib::json::parse(req.data()->body_as_string());
            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            auto app_user = get_app_user(cnx, app_id, app_user_id);
            fostlib::string reference;
            if (req.headers().exists("__odin_reference")) {
                reference = req.headers()["__odin_reference"].value();
            } else {
                reference = odin::reference();
            }
            if (app_user.isnull()) {
                auto identity_id = odin::reference();
                odin::create_user(cnx, identity_id);
                fostlib::json new_app_user;
                const fostlib::jcursor app_user_id_cursor("app_user_id");
                fostlib::insert(new_app_user, "app_id", app_id);
                fostlib::insert(new_app_user, "app_user_id", app_user_id);
                fostlib::insert(new_app_user, "identity_id", identity_id);
                fostlib::insert(new_app_user, "reference", reference);
                cnx.insert("odin.app_user_ledger", new_app_user);

                fostlib::json new_app_data;
                fostlib::insert(new_app_data, "app_id", app_id);
                fostlib::insert(new_app_data, "identity_id", identity_id);
                fostlib::insert(new_app_data, "app_data", app_data);
                fostlib::insert(new_app_data, "reference", reference);
                cnx.insert("odin.app_user_app_data_ledger", new_app_data);
            } else {
                fostlib::json new_app_data;
                fostlib::insert(new_app_data, "app_id", app_id);
                fostlib::insert(new_app_data, "identity_id", app_user["identity_id"]);
                fostlib::insert(new_app_data, "app_data", app_data);
                fostlib::insert(new_app_data, "reference", reference);
                cnx.insert("odin.app_user_app_data_ledger", new_app_data);
            }
            cnx.commit();



            auto jwt = odin::app::mint_user_jwt(
                    app_user_id,
                    app_id,
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
        }

    } c_app_user;
}

const fostlib::urlhandler::view &odin::view::app_user = c_app_user;
