cmake_minimum_required(VERSION 3.20)
project(Dev CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32)
  #set (FBX_LNK "C:/Program Files/Autodesk/FBX/FBX SDK/2019.0/lib/vs2015/x86/debug/")
  set (FBX_LNK "C:/Program Files/Autodesk/FBX/FBX SDK/2019.0/lib/vs2015/x86/")
  link_directories(${FBX_LNK})
endif (WIN32)

add_executable(${CMAKE_PROJECT_NAME} src/main.cpp)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/liblava ${CMAKE_CURRENT_BINARY_DIR}/liblava)
target_sources(
  ${PROJECT_NAME} PRIVATE
  src/main.cpp
)

if (WIN32)
  set (FBX_LIB "C:/Program Files/Autodesk/FBX/FBX SDK/2019.0/include")
  target_include_directories(${PROJECT_NAME} PRIVATE "${FBX_LIB}")
  set_property(TARGET ${PROJECT_NAME} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded")
  #set_property(TARGET ${PROJECT_NAME} PROPERTY
    #VS_USER_PROPS "libfbxsdk-mt.lib")
  #set_property(TARGET ${PROJECT_NAME} PROPERTY
    
endif (WIN32)

if (LINUX)
  find_library(FBX_LIB fbxsdk REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE "${FBX_LIB}")
endif (LINUX)

target_link_libraries(${PROJECT_NAME} PRIVATE lava::app)
