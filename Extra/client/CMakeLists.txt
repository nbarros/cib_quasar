

set(CMAKE_VERBOSE_MAKEFILE ON)


add_library(iols_functions EXCLUDE_FROM_ALL SHARED FeedbackManager.cpp Open62541Client.cpp  IoLSMonitor.cpp  toolbox.cpp open62541.c)
target_compile_definitions(iols_functions PUBLIC -DUA_LOGLEVEL=601)
target_compile_features(iols_functions PUBLIC cxx_std_17)
target_include_directories(iols_functions PUBLIC ${PROJECT_SOURCE_DIR}/Extra/client)
target_compile_options(iols_functions PUBLIC -Wno-literal-suffix -Wno-write-strings)

add_executable(iols_manager EXCLUDE_FROM_ALL iols_manager.cpp iols_manager_commands.cpp )
target_compile_features(iols_manager PUBLIC cxx_std_17)
target_include_directories(iols_manager PUBLIC ${PROJECT_SOURCE_DIR}/Extra/client)
target_compile_options(iols_manager PUBLIC -Wno-literal-suffix -Wno-write-strings)
target_link_libraries(iols_manager iols_functions pthread ncurses)


