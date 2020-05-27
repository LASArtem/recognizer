cmake_minimum_required(VERSION 3.5.1)

if (NOT DEFINED PRODUCT_VERSION)
    set(PRODUCT_VERSION "1.0.0")
endif()

project(Recognizer LANGUAGES CXX VERSION ${PRODUCT_VERSION})
set(EXEC_NAME "Recognizer${PROJECT_VERSION}.bin")
message("========== Start cmake : ${PROJECT_NAME} ${PROJECT_VERSION} will create ${EXEC_NAME}")

#----------------------------------------------------------------------------------------
message("= PROJECT_NAME      : ${PROJECT_NAME}")
message("= CMAKE_PROJECT_NAME: ${CMAKE_PROJECT_NAME}")
message("= PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message("= PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")

if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
endif()
message("= BIN_OUTPUT_DIR    : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

if (NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
endif()
message("= LIB_OUTPUT_DIR    : ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

if (NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
endif()
message("=== ARC_OUTPUT_DIR    : ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

#----------------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall")

include_directories(PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/CArticle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/CFileHolder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/CNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/tgnews_mgr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/tgparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/tools.cpp
)

add_library(ProjectLib ${SOURCES})

add_executable(project main.cpp)
target_link_libraries(project ProjectLib)

message("========== Finish cmake : ${PROJECT_NAME}")