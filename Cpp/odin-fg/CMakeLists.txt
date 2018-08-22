add_library(odin-fg
        native.cpp
        native.group.cpp
        native.jwt.cpp
        native.permission.cpp
        native.user.cpp
    )
target_link_libraries(odin-fg fostgres-fg odin)
set_target_properties(odin-fg PROPERTIES DEBUG_POSTFIX "-d")
install(TARGETS odin-fg LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

add_library(odin-fg-smoke STATIC EXCLUDE_FROM_ALL
        native.tests.cpp
    )
target_link_libraries(odin-fg-smoke odin-fg)
smoke_test(odin-fg-smoke)

