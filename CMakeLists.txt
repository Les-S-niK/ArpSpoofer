cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "ArpSpoofer")

project(${PROJECT_NAME} LANGUAGES CXX)

option(TESTING "Enable testing build" ON)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(TESTING)
  find_package(GTest REQUIRED)
  enable_testing()
  include(GoogleTest)
  add_executable(
    ArpSpooferTests
    tests/tests_frames.cpp tests/tests_net_interfaces.cpp
    ./tests/tests_sockets.cpp ./src/sockets.cpp ./src/net_interfaces.cpp)
  target_include_directories(ArpSpooferTests PRIVATE include)
  target_link_libraries(ArpSpooferTests PRIVATE GTest::gtest GTest::gtest_main)
  add_test(NAME ArpSpooferTests COMMAND ArpSpooferTests)

else()
  add_executable(${PROJECT_NAME} main.cpp)
  add_library(Src ./src/net_interfaces.cpp ./src/sockets.cpp)
  target_include_directories(
    ${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:include>)
  target_include_directories(Src PRIVATE include)
  target_link_libraries(${PROJECT_NAME} PRIVATE Src)
endif()
