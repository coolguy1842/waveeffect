cmake_minimum_required(VERSION 3.5.0)
project(wave VERSION 0.1.0 LANGUAGES CXX)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


include(CTest)
enable_testing()

link_directories(/usr/local/lib64/)
include_directories(include src/include)

add_executable(wave src/main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

IF (WIN32)
    target_link_libraries(wave hidapi)
ELSE()
    target_link_libraries(wave hidapi-hidraw)
ENDIF()
