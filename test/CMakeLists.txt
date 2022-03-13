cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project("unittesting")

###########
## GTEST ##
###########

enable_testing()
include(ExternalProject)

ExternalProject_Add(
  "gtest"
  URL "https://github.com/google/googletest/archive/refs/tags/release-1.10.0.zip"
  PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gtest"
  # Disable install step
  INSTALL_COMMAND ""
)

ExternalProject_Get_Property(gtest source_dir binary_dir)

add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)
include_directories("${source_dir}/googletest/include")
set_target_properties(libgtest PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/lib/libgtest.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)

add_library(libgtestmain IMPORTED STATIC GLOBAL)
add_dependencies(libgtestmain gtest)
set_target_properties(libgtestmain PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/lib/libgtest_main.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)

add_library(libgmock IMPORTED STATIC GLOBAL)
add_dependencies(libgmock gtest)
include_directories("${source_dir}/googlemock/include")
set_target_properties(libgmock PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/lib/libgmock.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)

###################
## CONFIGURATION ##
###################

add_definitions("-Wall -Werror -Wextra")

##############
## COVERAGE ##
##############

add_library(coverage_config INTERFACE)
target_compile_options(coverage_config INTERFACE "-fprofile-arcs" "-ftest-coverage")
target_link_libraries(coverage_config PUBLIC INTERFACE "--coverage")

###########
## TESTS ##
###########

include_directories("../src")

add_executable("bson_c"
  "bson_c.cpp"
  "../src/bson.c")
target_link_libraries("bson_c" "libgtestmain" "libgtest" "libgmock" "pthread" "coverage_config")
add_test(NAME "bson_c" COMMAND "bson_c" "${CMAKE_CURRENT_LIST_DIR}/data/data.bson")

add_executable("bson_cpp"
  "bson_cpp.cpp"
  "../src/bson.c"
  "../src/bson.cpp")
target_link_libraries("bson_cpp" "libgtestmain" "libgtest" "libgmock" "pthread" "coverage_config")
add_test(NAME "bson_cpp" COMMAND "bson_cpp" "${CMAKE_CURRENT_LIST_DIR}/data/data.bson")

