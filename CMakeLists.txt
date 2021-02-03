cmake_minimum_required(VERSION 3.0.0)
project(tus VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fopenmp -std=c++11 -Wall -Wextra -Werror")

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.71.0 COMPONENTS filesystem) 

find_package(OpenSSL REQUIRED)

include_directories(
  include
  ${Boost_INCLUDE_DIRS}
) 

include(CTest)
enable_testing()

add_executable(tus http_client_sync_ssl.cpp)
target_link_libraries(tus
  ${Boost_LIBRARIES}
  OpenSSL::SSL
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
