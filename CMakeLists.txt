cmake_minimum_required(VERSION 3.16)
project(DBMS)

set(CMAKE_CXX_STANDARD 14)

SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_VERBOSE_MAKEFILE false)

SET(SRC_DIR src/)
SET(LIB_DIR lib/)

SET(TARGET_SRC main.cpp)
SET(SRC_FOR_LIB
        ${SRC_DIR}Api.cpp
        ${SRC_DIR}DiskLayer.cpp
        ${SRC_DIR}IndexLayer.cpp
        ${SRC_DIR}BufferLayer.cpp
        ${SRC_DIR}TransactionLayer.cpp
        ${SRC_DIR}LockLayer.cpp
        ${SRC_DIR}LogLayer.cpp
        )

ADD_LIBRARY(bpt STATIC ${SRC_FOR_LIB})

include_directories(include)

add_executable(DBMS ${TARGET_SRC})
target_link_libraries(DBMS bpt)