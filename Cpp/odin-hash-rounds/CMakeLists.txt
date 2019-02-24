add_executable(odin-hash-rounds odin-hash-rounds.cpp)
target_link_libraries(odin-hash-rounds fost-cli odin)
install(TARGETS odin-hash-rounds EXPORT odin-hash-rounds RUNTIME DESTINATION bin)
