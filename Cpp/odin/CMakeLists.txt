add_library(odin
        app.cpp
        apple.cpp
        credentials.cpp
        facebook.cpp
        google.cpp
        group.cpp
        nonce.cpp
        odin.cpp
        pwhashproc.cpp
        thirdparty.cpp
        user.cpp
    )
target_include_directories(odin PUBLIC ../include)
target_link_libraries(odin fost-crypto fost-inet fost-postgres fostgres)
set_target_properties(odin PROPERTIES DEBUG_POSTFIX "-d")
install(TARGETS odin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(DIRECTORY ../include/odin DESTINATION include)

if(TARGET check)
    add_library(odin-smoke STATIC EXCLUDE_FROM_ALL
            credentials.tests.cpp
        )
    target_link_libraries(odin-smoke odin)
    smoke_test(odin-smoke)
endif()
