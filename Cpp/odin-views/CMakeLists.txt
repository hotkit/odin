add_library(odin-views
        login.cpp
        logout.cpp
        password.cpp
        permission.cpp
        registration.cpp
        secure.cpp
        user.cpp
    )
target_link_libraries(odin-views fost-urlhandler odin)
install(TARGETS odin-views LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

add_library(odin-views-smoke STATIC EXCLUDE_FROM_ALL
        login.tests.cpp
        secure.tests.cpp
    )
target_link_libraries(odin-views-smoke odin-views)
smoke_test(odin-views-smoke)
