install(FILES
        bootstrap.sql
    DESTINATION share/odin/Schema/)
install(DIRECTORY
        authn
        authz
        core
        opts
        app
    DESTINATION share/odin/Schema)
