## ----------------------------------------------------
## Please see Documentation/quasarBuildSystem.html for
## information how to use this file.
## ----------------------------------------------------

#set(CUSTOM_SERVER_MODULES AccessControl)
set(CUSTOM_SERVER_MODULES LaserControl LaserUtils)
include_directories( ${PROJECT_SOURCE_DIR}/LaserUtils/i2c/ )
include_directories( ${PROJECT_SOURCE_DIR}/LaserUtils/common/ )
add_subdirectory(${PROJECT_SOURCE_DIR}/Extra/client)
add_subdirectory(${PROJECT_SOURCE_DIR}/Extra/client/standalone_clients)

set(BUILD_SPDLOG_FMT_EXTERNAL OFF)

#include_directories( ${PROJECT_SOURCE_DIR}/LaserUtils/i2c/ )

set(EXECUTABLE OpcUaServer)
set(SERVER_INCLUDE_DIRECTORIES  )
set(SERVER_LINK_LIBRARIES  "-lcurl" spdlog i2c cib_i2c)
set(SERVER_LINK_DIRECTORIES  )

#add_compile_definitions(SIMULATION)


##set(CMAKE_VERBOSE_MAKEFILE ON)

##
## If ON, in addition to an executable, a shared object will be created.
##
set(BUILD_SERVER_SHARED_LIB OFF)

##
## Add here any additional boost libraries needed with their canonical name
## examples: date_time atomic etc.
## Note: boost paths are resolved either from $BOOST_ROOT if defined or system paths as fallback
##
set(ADDITIONAL_BOOST_LIBS )
