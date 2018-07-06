add_library(odin
        credentials.cpp
        nonce.cpp
        odin.cpp
        pwhashproc.cpp
        user.cpp
    )
target_include_directories(odin PUBLIC ../include)
target_link_libraries(odin fost-crypto fost-inet fost-postgres fostgres)
install(TARGETS odin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
