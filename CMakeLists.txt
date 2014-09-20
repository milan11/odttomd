project(odttomd)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wconversion -Wnon-virtual-dtor -Wswitch-default")
endif()

find_package(EXPAT REQUIRED)
include_directories(${EXPAT_INCLUDE_DIRS})
#find_package(ZIP REQUIRED)
set(ZIP_LIBRARIES "zip")
include_directories(${ZIP_INCLUDE_DIRS})

find_package(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${EXPAT_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${ZIP_LIBRARIES})
