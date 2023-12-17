include_directories(${PROTOBUF_INCLUDE_DIR})
execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/include --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/wib.proto)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-array-bounds")
add_library(CIBProto OBJECT src/cib.cc include/cib.pb.h include/cib.pb.cc include/cib.hh include/timer.hh)
