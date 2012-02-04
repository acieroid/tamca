cmake_minimum_required(VERSION 2.6)
project(tamca)

find_package(GTK2 REQUIRED)
find_package(OpenAL REQUIRED)
# find_package(ALUT REQUIRED) # not supported by cmake currently

set(CMAKE_CFLAGS "-g -Wall -ansi -pedantic")

add_executable(gtamca gtamca.c)
include_directories(gtamca ${GTK2_INCLUDE_DIRS} ${OPENAL_INCLUDE_DIR})
target_link_libraries(gtamca ${GTK2_LIBRARIES} ${OPENAL_LIBRARY} alut)
