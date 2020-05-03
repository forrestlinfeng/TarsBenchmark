
gen_server(tars tb)

include_directories(./)
include_directories(proto)
include_directories(licote)

add_library(tb_comm monitor.cpp transport.cpp licote/licote.c proto/tars_protocol.cpp)
