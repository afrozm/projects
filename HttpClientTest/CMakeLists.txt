cmake_minimum_required(VERSION 3.15)
project(HttpClientTest LANGUAGES CXX)

if(APPLE)
    enable_language(OBJCXX)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/../Common ${CMAKE_BINARY_DIR}/Common)

set(SOURCES HttpClientTest/HttpClientTest.cpp)

if(WIN32)
    list(APPEND SOURCES HttpClientTest/HttpClient.cpp)
elseif(APPLE)
    list(APPEND SOURCES HttpClientTest/HttpClient.mm)
endif()

add_executable(HttpClientTest ${SOURCES})

target_include_directories(HttpClientTest PRIVATE ${CMAKE_SOURCE_DIR}/HttpClientTest)

set_target_properties(HttpClientTest PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

target_compile_options(HttpClientTest PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall -Werror>
)

target_link_libraries(HttpClientTest PRIVATE Common)

if(WIN32)
    target_compile_definitions(HttpClientTest PRIVATE _CONSOLE UNICODE _UNICODE)
    target_link_libraries(HttpClientTest PRIVATE Wininet)
elseif(APPLE)
    set_source_files_properties(HttpClientTest/HttpClient.mm
        PROPERTIES COMPILE_FLAGS "-fobjc-arc"
    )
    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    target_link_libraries(HttpClientTest PRIVATE
        ${COCOA_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
    )
endif()
