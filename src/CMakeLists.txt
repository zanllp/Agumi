# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
aux_source_directory(. DIR_SRCS)

# 

if(DEFINED VCPKG_PATH)
  message("enable ssl")
  set(CMAKE_TOOLCHAIN_FILE ${VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake)
  project (agumi)
else()
  message("disable ssl")
  project (agumi)
endif()

add_executable(agumi ${DIR_SRCS})
target_link_libraries(agumi PRIVATE pthread)

if(DEFINED VCPKG_PATH)
  target_link_libraries(agumi PRIVATE OpenSSL::SSL OpenSSL::Crypto)
  find_package(OpenSSL REQUIRED)
else()
  add_definitions(-DSION_DISABLE_SSL)
endif()


set(CMAKE_BUILD_TYPE "Debug")


enable_testing()
add_test (test_run agumi -test)
set_tests_properties(test_run PROPERTIES ENVIRONMENT "WORKING_DIR_RELATIVE_PATH=..")
set_tests_properties (test_run
  PROPERTIES PASS_REGULAR_EXPRESSION "TestPassed;All ok")