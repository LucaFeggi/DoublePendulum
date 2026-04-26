cmake_minimum_required(VERSION 3.25)

cmake_policy(SET CMP0091 NEW)

project(DoublePendulum LANGUAGES C)

include(CheckIncludeFile)

if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "DoublePendulum requires GCC. Configure with one of the GCC Ninja presets.")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(DOUBLE_PENDULUM_OUTPUT_DIR "${CMAKE_BINARY_DIR}")

foreach(output_kind RUNTIME LIBRARY ARCHIVE PDB COMPILE_PDB)
    set(CMAKE_${output_kind}_OUTPUT_DIRECTORY "${DOUBLE_PENDULUM_OUTPUT_DIR}")
    foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
        set(CMAKE_${output_kind}_OUTPUT_DIRECTORY_${config} "${DOUBLE_PENDULUM_OUTPUT_DIR}")
    endforeach()
endforeach()

set(SDL2_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/externals/SDL2-2.32.10")

if(NOT EXISTS "${SDL2_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR "SDL2 submodule is missing. Run: git submodule update --init --recursive")
endif()

set(SDL_SHARED ON CACHE BOOL "Build SDL2 as a shared library" FORCE)
set(SDL_STATIC OFF CACHE BOOL "Do not build SDL2 as a static library" FORCE)
set(SDL_TEST OFF CACHE BOOL "Do not build SDL2_test" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Do not build SDL2 tests" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "Do not install SDL2 from this project" FORCE)
add_subdirectory("${SDL2_ROOT}" "${CMAKE_BINARY_DIR}/externals/SDL2" EXCLUDE_FROM_ALL)

check_include_file(threads.h DOUBLE_PENDULUM_HAVE_C11_THREADS)

set(DOUBLE_PENDULUM_RENDERER_BACKEND "sdl" CACHE STRING "Renderer backend selected at build time")
set_property(CACHE DOUBLE_PENDULUM_RENDERER_BACKEND PROPERTY STRINGS sdl)

if(NOT DOUBLE_PENDULUM_RENDERER_BACKEND STREQUAL "sdl")
    message(FATAL_ERROR "Only the sdl renderer backend is available right now.")
endif()

set(DOUBLE_PENDULUM_RENDERER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/${DOUBLE_PENDULUM_RENDERER_BACKEND}")

add_executable(DoublePendulum
    src/main.c
    src/app/app.c
    src/app/fps.c
    src/app/input.c
    src/app/render_data.c
    src/app/threadpool.c
    src/app/window.c
    "${DOUBLE_PENDULUM_RENDERER_DIR}/color.c"
    "${DOUBLE_PENDULUM_RENDERER_DIR}/renderer.c"
    "${DOUBLE_PENDULUM_RENDERER_DIR}/trail.c"
    src/simulation/integrators/rk4.c
    src/simulation/pendulum.c
    src/simulation/pendulum_equations.c
    src/simulation/rod.c
    src/simulation/simulation.c
)

target_include_directories(DoublePendulum PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/app"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/renderer"
    "${DOUBLE_PENDULUM_RENDERER_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/simulation"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/simulation/integrators"
)

target_compile_definitions(DoublePendulum PRIVATE SDL_MAIN_HANDLED)

if(NOT DOUBLE_PENDULUM_HAVE_C11_THREADS)
    message(FATAL_ERROR "DoublePendulum requires a GCC toolchain with C11 <threads.h> support.")
endif()

target_link_libraries(DoublePendulum PRIVATE SDL2::SDL2)
find_package(Threads REQUIRED)
target_link_libraries(DoublePendulum PRIVATE Threads::Threads)

if(UNIX AND NOT APPLE)
    target_link_libraries(DoublePendulum PRIVATE m)
endif()

target_compile_options(DoublePendulum PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

set_target_properties(DoublePendulum PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${DOUBLE_PENDULUM_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${DOUBLE_PENDULUM_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${DOUBLE_PENDULUM_OUTPUT_DIR}"
    PDB_OUTPUT_DIRECTORY "${DOUBLE_PENDULUM_OUTPUT_DIR}"
    COMPILE_PDB_OUTPUT_DIRECTORY "${DOUBLE_PENDULUM_OUTPUT_DIR}"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
