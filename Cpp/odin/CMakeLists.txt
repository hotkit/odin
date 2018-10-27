add_library(odin
        app.cpp
        credentials.cpp
        facebook.cpp
        google.cpp
        group.cpp
        nonce.cpp
        odin.cpp
        pwhashproc.cpp
        user.cpp
    )
target_include_directories(odin PUBLIC ../include)
target_link_libraries(odin fost-crypto fost-inet fost-postgres fostgres)
set_target_properties(odin PROPERTIES DEBUG_POSTFIX "-d")
install(TARGETS odin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
