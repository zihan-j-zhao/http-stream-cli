# CMakeList.txt : CMake project for http-stream-cli, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

# Set cpp and header files
set(SOURCES
        "src/http-stream-cli.cpp"
)
set(HEADERS
        "include/http-stream-cli.h"
        "include/utils.h"
        "include/parser/validator.h"
        "include/source/source.h"
        "include/source/rand_source.h"
        "include/source/xlsx_source.h"
)

set(NLOHMJSON_LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nlohmann-json/include)
set(SPDLOG_LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/spdlog/include)
set(DATE_LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/date/include)

# Set third-party Excel library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/OpenXLSX)

# Add source to this project's executable.
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} )
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE include)
target_include_directories(${CMAKE_PROJECT_NAME} 
        PRIVATE
        ${NLOHMJSON_LIB_HEADERS}
        ${SPDLOG_LIB_HEADERS}
        ${DATE_LIB_HEADERS}
)
target_link_libraries(${CMAKE_PROJECT_NAME} 
        PRIVATE
        OpenXLSX::OpenXLSX
)

# TODO: Add tests and install targets if needed.
