cmake_minimum_required(VERSION 3.20)
project(Dev CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ENABLE_EXPORTS ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_executable(${CMAKE_PROJECT_NAME} src/main.cpp ${BACKWARD_ENABLE})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/backward-cpp)
  add_backward(${CMAKE_PROJECT_NAME})
else()
  add_executable(${CMAKE_PROJECT_NAME} src/main.cpp)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/liblava ${CMAKE_CURRENT_BINARY_DIR}/liblava)

target_sources(
  ${PROJECT_NAME} PRIVATE
  src/main.cpp
  src/includes.h
  src/fbx_loading.h
  src/fbx_loading.cpp
  src/pipelines.h
  src/pipelines.cpp
)

# if(WIN32)
#   set(FBX_LIB "C:/Program Files/Autodesk/FBX/FBX SDK/2019.0/include")
#   target_include_directories(${PROJECT_NAME} PRIVATE "${FBX_LIB}")
#   set_property(
#     TARGET ${PROJECT_NAME} PROPERTY
#     MSVC_RUNTIME_LIBRARY "MultiThreaded"
#   )
#   #set_property(TARGET ${PROJECT_NAME} PROPERTY
#   #VS_USER_PROPS "libfbxsdk-mt.lib")
#   #set_property(TARGET ${PROJECT_NAME} PROPERTY
# endif(WIN32)

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

if(LINUX)
  set(FBX_LIB "${CMAKE_CURRENT_SOURCE_DIR}/fbxsdk")
  target_include_directories(${PROJECT_NAME} PUBLIC "${FBX_LIB}/include" REQUIRED)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(${PROJECT_NAME} PUBLIC "${FBX_LIB}/lib/gcc/x64/debug/libfbxsdk.a")
    message("DEBUG")
  else()
    target_link_libraries(${PROJECT_NAME} PUBLIC "${FBX_LIB}/lib/gcc/x64/release/libfbxsdk.a")
    message("RELEASE")
  endif()
endif(LINUX)

target_link_libraries(${PROJECT_NAME} PRIVATE lava::app)
