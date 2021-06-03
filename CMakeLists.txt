cmake_minimum_required(VERSION 3.20)
project(Dev CXX)

add_executable(${CMAKE_PROJECT_NAME} src/main.cpp)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/liblava ${CMAKE_CURRENT_BINARY_DIR}/liblava)
target_sources(
  ${PROJECT_NAME} PRIVATE
  src/main.cpp
)

find_library(FBX_LIB fbxsdk REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE lava::app)
target_link_libraries(${PROJECT_NAME} PRIVATE "${FBX_LIB}")
