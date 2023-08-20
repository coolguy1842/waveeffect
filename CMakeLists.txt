cmake_minimum_required(VERSION 3.5.0)
project(wave VERSION 0.1.0 LANGUAGES CXX)

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
