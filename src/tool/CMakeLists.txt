gen_server(tars tb)

add_dependencies(tb tb_comm)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_link_libraries(tb -Wl,-all_load tb_comm -Wl,-noall_load)
else
    target_link_libraries(tb -Wl,--whole-archive tb_comm -Wl,--no-whole-archive)
endif()


target_link_libraries(tb tarsutil)
