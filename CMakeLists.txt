cmake_minimum_required (VERSION 2.8)
################################################################################
set(CMAKE_BUILD_TYPE Debug)
set(GLOB BOOST_VERSION 1.55)

################################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/i2pd/build)

################################################################################
project(chat)

find_package(Boost ${BOOST_VERSION} COMPONENTS filesystem
                                               program_options
                                               thread
                                               system
                                               coroutine
                                               REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -ggdb ${SANITIZE}")

include_directories(
    "${Boost_INCLUDE_DIR}"
    "${CMAKE_SOURCE_DIR}/include/"
    "${CMAKE_SOURCE_DIR}/i2pd/libi2pd"
    "${CMAKE_SOURCE_DIR}/i2pd/libi2pd_client")

file(GLOB sources
    "${CMAKE_SOURCE_DIR}/src/channel.cpp"
    "${CMAKE_SOURCE_DIR}/src/service.cpp"
    "${CMAKE_SOURCE_DIR}/example/chat.cpp")

add_executable(chat ${sources})
add_dependencies(chat i2pd)

target_link_libraries(chat
    i2pdclient
    libi2pd
    ssl
    crypto
    z
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})

