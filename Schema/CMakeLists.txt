install(FILES
        bootstrap.sql
    DESTINATION share/odin/Schema/)
install(DIRECTORY
        app
        authn
        authz
        core
        opts
    DESTINATION share/odin/Schema)
