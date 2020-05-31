include_directories(comm)
include_directories(comm/proto)
include_directories(comm/licote)
include_directories(comm/tarsparser)

add_subdirectory(comm)
add_subdirectory(tool)
add_subdirectory(server/NodeServer)
add_subdirectory(server/AdminServer)

