cmake_minimum_required(VERSION 3.15)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment target")
project(SalT LANGUAGES CXX)

if(APPLE)
    enable_language(OBJCXX)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/../Common ${CMAKE_BINARY_DIR}/Common)
add_subdirectory(${CMAKE_SOURCE_DIR}/../CommonXplat ${CMAKE_BINARY_DIR}/CommonXplat)

set(SOURCES
    SalT/ImgMgmtCommon.cpp
)

if(WIN32)
    list(APPEND SOURCES
        SalT/SalT.cpp
        SalT/cMD5.cpp
        SalT/ImgMgmt.cpp
        SalT/Path.cpp
        SalT/RegisterSelf.cpp
        SalT/SalT.rc
    )
elseif(APPLE)
    list(APPEND SOURCES
        SalT/main.cpp
        SalT/MacUtil.mm
        SalT/ImgMgmtMac.mm
    )
endif()

add_executable(SalT ${SOURCES})

if(APPLE)
    target_include_directories(SalT BEFORE PRIVATE
        ${CMAKE_SOURCE_DIR}/../Common
        ${CMAKE_SOURCE_DIR}/../CommonXplat
    )
endif()

target_include_directories(SalT PRIVATE ${CMAKE_SOURCE_DIR}/SalT)

set_target_properties(SalT PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

target_compile_options(SalT PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall -Werror>
)

if(WIN32)
    target_compile_definitions(SalT PRIVATE WIN32 _WINDOWS UNICODE _UNICODE)
    set_target_properties(SalT PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_libraries(SalT PRIVATE shlwapi Gdiplus Version Psapi)
elseif(APPLE)
    set_source_files_properties(SalT/MacUtil.mm SalT/ImgMgmtMac.mm
        PROPERTIES COMPILE_FLAGS "-fobjc-arc"
    )
    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
    find_library(APPLICATION_SERVICES_FRAMEWORK ApplicationServices REQUIRED)
    target_link_libraries(SalT PRIVATE
        CommonXplat
        ${COCOA_FRAMEWORK}
        ${APPLICATION_SERVICES_FRAMEWORK}
    )
endif()
