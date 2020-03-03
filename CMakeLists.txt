##
cmake_minimum_required(VERSION 3.2)

project(PidExtractor
  VERSION 0.0.5
)

add_executable(PidExtractor
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_sources(PidExtractor
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TsFile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TsDbg.cpp
)

target_include_directories(PidExtractor
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(PidExtractor
  PRIVATE
    cxx_std_11
)
