cmake_minimum_required(VERSION 3.20)

project(bgfx-imgui-example VERSION 1.0)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

option(STATIC_BUILD "Build with static linking" ON)
option(USE_FULL_STATIC "Attempt full static linking (may not work on all platforms)" OFF)

set(CMAKE_CXX_STANDARD 17)
set(SDL_STATIC ON)
set(SDL_SHARED OFF)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin/${PROJECT_NAME})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin/${PROJECT_NAME})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/libs)

# Static runtime
if(MSVC OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(MINGW OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND WIN32))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    if(STATIC_BUILD)
        if(APPLE)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
        elseif(USE_FULL_STATIC)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
        else()
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
        endif()
    endif()
elseif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    if(USE_FULL_STATIC)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
    endif()
endif()

add_subdirectory(libs/bgfx.cmake)
add_subdirectory(libs/SDL3)

file(GLOB imgui
    libs/imgui/*.cpp
    libs/imgui/backends/imgui_impl_sdl3.cpp
    src/imgui_impl_bgfx.cpp
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/icon.rc
    ${imgui}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    libs/imgui
    libs/imgui/backends
    libs/bgfx.cmake/bgfx/include
    libs/bgfx.cmake/bimg/include
    libs/bgfx.cmake/bx/include
    libs/SDL3/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    SDL3::SDL3-static
    bgfx
)

# bx:When using MSVC you must set /Zc:__cplusplus compiler option.
if (MSVC)
    target_compile_options(${PROJECT_NAME}
    PUBLIC /Zc:__cplusplus 
    PUBLIC /Zc:preprocessor)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE BX_CONFIG_DEBUG=1)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE BX_CONFIG_DEBUG=1)
endif()
if (LINUX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED)
endif()

add_custom_command(
    TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND echo ${PROJECT_NAME}: Copying Resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/res  ${PROJECT_SOURCE_DIR}/build/bin/${PROJECT_NAME}/${CMAKE_CFG_INTDIR}
)
