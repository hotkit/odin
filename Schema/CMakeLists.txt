install(FILES
        bootstrap.sql
    DESTINATION share/odin/schema/)
install(DIRECTORY
        authn
        authz
        core
        opts
    DESTINATION share/odin/schema)
