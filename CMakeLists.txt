## 
cmake_minimum_required(VERSION 3.2)

project(PidExtractor)

#
# Directory with modules
#
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

#
# Configuration variables
#
option(INSTALL_HEADERS "Install the headers" OFF)
option(INSTALL_CLIENT "Install the Client" OFF)

#
# Extra compiler / linker options
#
IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -Wformat=2")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#
# Packages to use
#
#find_package(Threads REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#
# Sources to compile
#
set(REPO_SOURCES
    src/main.cpp
    src/TsFile.cpp
    src/TsDbg.cpp
)

#
# Targets
#
add_executable(pidex ${REPO_SOURCES})

set_property(TARGET pidex PROPERTY CXX_STANDARD 11)

#target_link_libraries(${PROGRAM_NAME} Threads::Threads)
