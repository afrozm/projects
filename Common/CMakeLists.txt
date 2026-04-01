if(NOT TARGET Common)

cmake_minimum_required(VERSION 3.15)

if(APPLE)
    enable_language(OBJCXX)
endif()

set(COMMON_SOURCES
    BinaryFind.cpp
    cMD5.cpp
    ProcessUtilX.cpp
    Progress.cpp
    StdCondVar.cpp
    StringUtils.cpp
    Value.cpp
)

if(WIN32)
    list(APPEND COMMON_SOURCES
        CountTimer.cpp
        DataReader.cpp
        FileMapping.cpp
        Path.cpp
        ProcessUtil.cpp
        Property.cpp
        StdConsole.cpp
        StdUtils.cpp
        TextReader.cpp
    )
elseif(APPLE)
    list(APPEND COMMON_SOURCES
        Path.mm
        ProcessUtil.mm
    )
endif()

list(TRANSFORM COMMON_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

add_library(Common STATIC ${COMMON_SOURCES})

target_include_directories(Common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(Common PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

target_compile_options(Common PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:
        -Wall -Werror
        -fvisibility=hidden -fvisibility-inlines-hidden
    >
)

if(APPLE)
    set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/Path.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/ProcessUtil.mm
        PROPERTIES COMPILE_FLAGS "-fobjc-arc"
    )
endif()

endif() # NOT TARGET Common
