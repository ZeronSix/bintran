cmake_minimum_required(VERSION 3.5)
project(bintran)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++1z")

set(ZVM_SOURCES zvm.cpp exceptions.hpp zvmarch.hpp datatools.cpp)
set(BINTRAN_SOURCES bintran_main.cpp bintran.cpp zvmarch.hpp
                    datatools.cpp bintran_x86arch.cpp)
set(ZASM_SOURCES zasm.cpp exceptions.hpp zvmarch.hpp datatools.cpp)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(zvm ${ZVM_SOURCES})
target_link_libraries(zvm stdc++fs)

add_executable(zasm ${ZASM_SOURCES})
target_link_libraries(zasm stdc++fs)

add_executable(bintran ${BINTRAN_SOURCES})
target_link_libraries(bintran stdc++fs)
