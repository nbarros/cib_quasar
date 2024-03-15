#include_directories(${PROTOBUF_INCLUDE_DIR})
#execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/include --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/wib.proto)
#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-array-bounds")

# We need c++11
set(CMAKE_CXX_STANDARD 11 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#include_directories(include)
#set(LIB_SRC_FILES src/Attenuator.cpp ${PROJECT_SOURCE_DIR}/src/Device.cpp ${PROJECT_SOURCE_DIR}/src/Laser.cpp ${PROJECT_SOURCE_DIR}/src/PowerMeter.cpp ${PROJECT_SOURCE_DIR}/src/serial.cc ${PROJECT_SOURCE_DIR}/src/utilities.cpp)

add_executable(test_system_load EXCLUDE_FROM_ALL test_system_load.cpp)
#target_link_libraries(test_all_devices PUBLIC LaserControl)
