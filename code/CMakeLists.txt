cmake_minimum_required(VERSION 3.5)
project (rhex)
find_package (Threads)
set(SOURCES
   src/Cando_DJIMotor_test.cpp
   )
add_executable(rhex ${SOURCES})
target_include_directories(rhex
  PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries (rhex Threads::Threads)