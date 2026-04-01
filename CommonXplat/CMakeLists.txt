if(NOT TARGET CommonXplat)

cmake_minimum_required(VERSION 3.15)

set(COMMONXPLAT_SOURCES
    Logger.cpp
    SingletonManager.cpp
)

list(TRANSFORM COMMONXPLAT_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

add_library(CommonXplat STATIC ${COMMONXPLAT_SOURCES})

target_include_directories(CommonXplat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(CommonXplat PUBLIC Common)

set_target_properties(CommonXplat PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

target_compile_options(CommonXplat PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:
        -Wall -Wno-unknown-pragmas -Wno-reorder -Werror
        -fvisibility=hidden -fvisibility-inlines-hidden
    >
)

endif() # NOT TARGET CommonXplat
