cmake_minimum_required(VERSION 2.8)

project(bm-ProxyServer)

gen_server(bm ProxyServer)

include_directories(../NodeServer)
add_dependencies(ProxyServer NodeServer tb_comm)

target_link_libraries(ProxyServer tb_comm)
#target_link_libraries(mysqlclient ssl crypto nghttp2-static)
