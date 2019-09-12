/**
    Copyright 2016-2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <odin/odin.hpp>
#include <odin/views.hpp>

#include <fost/insert>
#include <fostgres/sql.hpp>


namespace {


    const class permission : public fostlib::urlhandler::view {
      public:
        static const fostlib::jcursor userloc;

        permission() : view("odin.permission") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            const auto &permission = config["permission"];
            bool granted = false;
            if (odin::c_jwt_trust.value() && req.headers().exists("__jwt")) {
                const auto &perm_header = req.headers()["__jwt"].subvalue(
                        odin::c_jwt_permissions_claim.value());
                if (perm_header) {
                    auto perms = fostlib::json::parse(perm_header.value());
                    for (const auto &p : perms) {
                        if (p == permission) {
                            granted = true;
                            break;
                        }
                    }
                }
            }
            if (not granted) {
                fostlib::pg::connection cnx{fostgres::connection(config, req)};
                fostlib::json where;
                if (not req[userloc]) {
                    throw fostlib::exceptions::not_implemented(
                            __func__,
                            "The odin.permission view must be wrapped by an "
                            "odin.secure "
                            "view on the secure path so that there is a valid "
                            "JWT to find "
                            "the user ID in");
                }
                fostlib::insert(where, "identity_id", req[userloc].value());
                fostlib::insert(where, "permission_slug", permission);
                auto rs = cnx.select("odin.user_permission", where);
                granted = rs.begin() != rs.end();
            }
            if (granted) {
                return execute(config["allowed"], path, req, host);
            } else {
                return execute(config["forbidden"], path, req, host);
            }
        }
    } c_permission;


    const fostlib::jcursor permission::userloc("headers", "__user");


}
