namespace {

    fostlib::json parse_payload(fostlib::http::server::request &req) {
        // TODO: Support multiple ContentType
        auto body_str = fostlib::coerce<fostlib::string>(
                fostlib::coerce<fostlib::utf8_string>(req.data()->data()));
        return fostlib::json::parse(body_str);
    }


    const class app_login : public fostlib::urlhandler::view {
      public:
        app_login() : view("odin.app.login") {}

        std::pair<boost::shared_ptr<fostlib::mime>, int> operator()(
                const fostlib::json &config,
                const fostlib::string &path,
                fostlib::http::server::request &req,
                const fostlib::host &host) const {
            const auto paths = fostlib::split(path, "/");
            if (paths.size() != 1)
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__, "Must pass app_id in the URL");
            fostlib::pg::connection cnx{fostgres::connection(config, req)};
            const auto app_id = paths[0];
            fostlib::json app = odin::app::get_detail(cnx, app_id);
            cnx.commit();
            if (app.isnull()) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__, "App not found");
            }

            if (req.method() == "GET") {
                boost::filesystem::wpath filename(
                        fostlib::coerce<boost::filesystem::wpath>(
                                config["static"]));
                return fostlib::urlhandler::serve_file(config, req, filename);
            }
            if (req.method() != "POST")
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "App Login required POST, this should be a 405");

            fostlib::json body = parse_payload(req);
            if (!body.has_key("username") || !body.has_key("password")) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__,
                        "Must pass both username and password fields");
            }
            const auto username =
                    fostlib::coerce<fostlib::string>(body["username"]);
            const auto password =
                    fostlib::coerce<fostlib::string>(body["password"]);


            auto user = odin::credentials(
                    cnx, username, password, req.remote_address());
            if (user.isnull()) {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__, "User not found");
            }
            auto const access_policy = fostlib::coerce<fostlib::string>(
                    app["app"]["access_policy"]);
            auto const identity_id =
                    fostlib::coerce<f5::u8view>(user["identity"]["id"]);
            auto app_user = odin::app::get_app_user(cnx, app_id, identity_id);
            if (app_user.isnull()) {
                if (access_policy == "INVITE_ONLY") {
                    throw fostlib::exceptions::not_implemented(
                            __PRETTY_FUNCTION__, "Forbidden");
                } else if (access_policy == "OPEN") {
                    odin::app::save_app_user(
                            cnx, odin::reference(), identity_id, app_id);
                    cnx.commit();
                }
            }

            auto jwt = odin::app::mint_user_jwt(
                    identity_id, app_id,
                    fostlib::coerce<fostlib::timediff>(config["expires"]));
            const auto redirect_url = fostlib::coerce<fostlib::string>(
                    app["app"]["redirect_url"]);
            fostlib::json result;
            fostlib::insert(result, "access_token", jwt.first);
            fostlib::insert(result, "scheme", "Bearer");
            fostlib::insert(result, "redirect_url", redirect_url);
            fostlib::mime::mime_headers headers;
            headers.add(
                    "Expires",
                    fostlib::coerce<fostlib::rfc1123_timestamp>(jwt.second)
                            .underlying()
                            .underlying());
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    fostlib::json::unparse(result, true), headers,
                    L"application/json"));
            return std::make_pair(response, 200);
        }

    } c_app_login;


}