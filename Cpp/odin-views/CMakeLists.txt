add_library(odin-views
        app_installation.cpp
        app_secure.cpp
        app_user.cpp
        app.cpp
        app.facebook.cpp
        app.google.cpp
        app.mint.cpp
        facebook.cpp
        google.cpp
        jwt.renewal.cpp
        login.cpp
        logout.cpp
        middleware.reference.cpp
        password.cpp
        permission.cpp
        registration.cpp
        secure.cpp
        user.cpp
    )
target_link_libraries(odin-views fost-urlhandler odin)
set_target_properties(odin-views PROPERTIES DEBUG_POSTFIX "-d")
install(TARGETS odin-views LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

if(TARGET check)
    add_library(odin-views-smoke STATIC EXCLUDE_FROM_ALL
            login.tests.cpp
            middleware.reference.tests.cpp
            password.hash.tests.cpp
            secure.tests.cpp
            jwt.renewal.tests.cpp
        )
    target_link_libraries(odin-views-smoke odin-views)
    smoke_test(odin-views-smoke)
endif()
