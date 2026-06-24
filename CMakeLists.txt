cmake_minimum_required(VERSION 3.14)
project(errno_names LANGUAGES CXX)

add_library(errno_names INTERFACE)
target_include_directories(errno_names INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(errno_names INTERFACE cxx_std_17)

enable_testing()
add_executable(errno_names_test test/test.cc)
target_link_libraries(errno_names_test PRIVATE errno_names)
add_test(NAME errno_names COMMAND errno_names_test)
