cmake_minimum_required(VERSION 2.8)

project(bm-NodeServer)

gen_server(bm NodeServer)

add_dependencies(NodeServer tb_comm)

target_link_libraries(NodeServer tb_comm)
#target_link_libraries(mysqlclient ssl crypto nghttp2-static)
