install(FILES
        app.register.json
    DESTINATION share/odin/views/)

if(TARGET stress OR TARGET pgtest)
    add_custom_command(OUTPUT test-view-app.register
            COMMAND fostgres-test odin-test-view-app.register -o test-view-app.register
                $<TARGET_SONAME_FILE:odin-fg>
                $<TARGET_SONAME_FILE:odin-views>
                ${CMAKE_CURRENT_SOURCE_DIR}/../Schema/bootstrap.sql
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-views.json
                ${CMAKE_CURRENT_SOURCE_DIR}/../Configuration/odin-federation-server.json
                ${CMAKE_CURRENT_SOURCE_DIR}/app.register.json
                ${CMAKE_CURRENT_SOURCE_DIR}/app.register.test.json
                ${CMAKE_CURRENT_SOURCE_DIR}/app.register.fg
            MAIN_DEPENDENCY app.register.fg
            DEPENDS
                fostgres-test
                fostgres
                odin-fg
                odin-views
                ../Configuration/odin-views.json
                ../Configuration/odin-federation-server.json
                ../Schema
                ../Schema/bootstrap.sql
                ../Schema/core/000-initial.blue.sql
                ../Schema/authn/001-initial.blue.sql
                ../Schema/authn/002-fix-login.blue.sql
                ../Schema/authz/001-initial.blue.sql
                ../Schema/authz/002-view-user_permission.blue.sql
                app.register.json
                app.register.fg
        )

    ## Because of the way cmake works we need this stuff at the end to
    ## actually make the above commands run when things change.
    add_custom_target(odin-view-tests DEPENDS
            test-view-app.register
        )
    if(TARGET stress)
        add_dependencies(stress odin-view-tests)
    endif()
    if(TARGET pgtest)
        add_dependencies(pgtest odin-view-tests)
    endif()
endif()
