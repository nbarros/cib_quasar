

set(CMAKE_VERBOSE_MAKEFILE ON)

#find_package(spdlog REQUIRED)
#find_package(i2c REQUIRED)
#set(BUILD_SPDLOG_FMT_EXTERNAL OFF)

find_package(spdlog)
add_executable(cib_sc_client_system EXCLUDE_FROM_ALL cib_sc_client_system.cpp open62541.c)
target_compile_features(cib_sc_client_system PUBLIC cxx_std_11)
target_include_directories(cib_sc_client_system PUBLIC ${PROJECT_SOURCE_DIR}/Extra/client)
target_compile_options(cib_sc_client_system PUBLIC -Wno-literal-suffix -Wno-write-strings)
if(${BUILD_SPDLOG_FMT_EXTERNAL})
	target_link_libraries(cib_sc_client_system spdlog fmt)
	target_compile_definitions(cib_sc_client_system PUBLIC SPDLOG_FMT_EXTERNAL) 
else()
	target_link_libraries(cib_sc_client_system spdlog)
endif(${BUILD_SPDLOG_FMT_EXTERNAL})

add_executable(cib_browse_nodes EXCLUDE_FROM_ALL cib_sc_browse_nodes.cpp)
target_compile_features(cib_browse_nodes PUBLIC cxx_std_11)
target_include_directories(cib_browse_nodes PUBLIC ${PROJECT_SOURCE_DIR}/Extra/client)
target_compile_options(cib_browse_nodes PUBLIC -Wno-literal-suffix -Wno-write-strings)
if(${BUILD_SPDLOG_FMT_EXTERNAL})
	target_link_libraries(cib_browse_nodes spdlog fmt)
	target_compile_definitions(cib_browse_nodes PUBLIC SPDLOG_FMT_EXTERNAL) 
else()
	target_link_libraries(cib_browse_nodes spdlog)
endif(${BUILD_SPDLOG_FMT_EXTERNAL})


add_library(iols_functions EXCLUDE_FROM_ALL SHARED FeedbackManager.cpp Open62541Client.cpp  IoLSMonitor.cpp  toolbox.cpp open62541.c)
target_compile_definitions(iols_functions PUBLIC -DUA_LOGLEVEL=601)
target_compile_features(iols_functions PUBLIC cxx_std_17)
target_include_directories(iols_functions PUBLIC ${PROJECT_SOURCE_DIR}/Extra/client)
target_compile_options(iols_functions PUBLIC -Wno-literal-suffix -Wno-write-strings)

add_executable(iols_manager EXCLUDE_FROM_ALL iols_manager.cpp)
target_compile_features(iols_manager PUBLIC cxx_std_17)
target_include_directories(iols_manager PUBLIC ${PROJECT_SOURCE_DIR}/Extra/client)
target_compile_options(iols_manager PUBLIC -Wno-literal-suffix -Wno-write-strings)
target_link_libraries(iols_manager iols_functions pthread ncurses)


