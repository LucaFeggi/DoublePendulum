cmake_minimum_required(VERSION 3.25)

project(DoublePendulum LANGUAGES C)

include(CheckCSourceCompiles)
include(CheckIPOSupported)
include(CMakePushCheckState)

set(DP_SUPPORTED_PLATFORMS win32 posix)
set(DP_SUPPORTED_INTEGRATORS rk4)
set(DP_SUPPORTED_RENDERERS sdl)

if(NOT CMAKE_GENERATOR MATCHES "^Ninja")
    message(FATAL_ERROR "DoublePendulum requires the Ninja generator. Configure with one of the dp-* presets.")
endif()

if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "DoublePendulum requires GCC. Configure with one of the dp-* presets.")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(DP_NATIVE_PLATFORM win32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(DP_NATIVE_PLATFORM posix)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}. Supported native platforms are Windows and Linux.")
endif()

set(DP_PLATFORM "${DP_NATIVE_PLATFORM}" CACHE STRING "Native platform selected at configure time")
set_property(CACHE DP_PLATFORM PROPERTY STRINGS ${DP_SUPPORTED_PLATFORMS})

set(DP_INTEGRATOR "rk4" CACHE STRING "Integrator selected at compile time")
set_property(CACHE DP_INTEGRATOR PROPERTY STRINGS ${DP_SUPPORTED_INTEGRATORS})

set(DP_RENDERER "sdl" CACHE STRING "Renderer selected at compile time")
set_property(CACHE DP_RENDERER PROPERTY STRINGS ${DP_SUPPORTED_RENDERERS})

if(NOT DP_PLATFORM IN_LIST DP_SUPPORTED_PLATFORMS)
    message(FATAL_ERROR "Unsupported DP_PLATFORM '${DP_PLATFORM}'. Supported values: ${DP_SUPPORTED_PLATFORMS}")
endif()

if(NOT DP_PLATFORM STREQUAL DP_NATIVE_PLATFORM)
    message(FATAL_ERROR "Preset platform '${DP_PLATFORM}' does not match host platform '${DP_NATIVE_PLATFORM}'.")
endif()

if(NOT DP_INTEGRATOR IN_LIST DP_SUPPORTED_INTEGRATORS)
    message(FATAL_ERROR "Unsupported DP_INTEGRATOR '${DP_INTEGRATOR}'. Supported values: ${DP_SUPPORTED_INTEGRATORS}")
endif()

if(NOT DP_RENDERER IN_LIST DP_SUPPORTED_RENDERERS)
    message(FATAL_ERROR "Unsupported DP_RENDERER '${DP_RENDERER}'. Supported values: ${DP_SUPPORTED_RENDERERS}")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(NOT CMAKE_CONFIGURATION_TYPES)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(DP_RUNTIME_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
set(DP_LIBRARY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/lib")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DP_RUNTIME_OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${DP_LIBRARY_OUTPUT_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${DP_LIBRARY_OUTPUT_DIR}")

foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config} "${DP_RUNTIME_OUTPUT_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config} "${DP_LIBRARY_OUTPUT_DIR}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config} "${DP_LIBRARY_OUTPUT_DIR}")
endforeach()

set(DP_C11_THREADS_REQUIRED_INCLUDES)
set(DP_C11_THREADS_REQUIRED_LIBRARIES)
set(DP_C11_THREADS_CHECK_IMPLEMENTATION)
set(DP_C11_THREADS_TARGET_INCLUDES)
set(DP_C11_THREADS_TARGET_SOURCES)
set(DP_USE_MINGW_C11THREADS OFF)

if(WIN32 AND MINGW)
    # MinGW-w64 does not reliably provide C11 <threads.h>; use c11threads as a generated build-tree compatibility header.
    set(DP_USE_MINGW_C11THREADS ON)
    set(DP_C11THREADS_GIT_TAG "master" CACHE STRING "jtsiomb/c11threads revision with native Win32 support")
    if(DP_C11THREADS_GIT_TAG STREQUAL "e3d46ff")
        set(DP_C11THREADS_GIT_TAG "master" CACHE STRING "jtsiomb/c11threads revision with native Win32 support" FORCE)
    endif()

    include(FetchContent)
    FetchContent_Declare(c11threads
        GIT_REPOSITORY https://github.com/jtsiomb/c11threads.git
        GIT_TAG ${DP_C11THREADS_GIT_TAG}
    )
    FetchContent_MakeAvailable(c11threads)

    set(DP_C11THREADS_GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated/include")
    file(MAKE_DIRECTORY "${DP_C11THREADS_GENERATED_INCLUDE_DIR}")
    file(WRITE "${DP_C11THREADS_GENERATED_INCLUDE_DIR}/threads.h" "#pragma once\n#include <c11threads.h>\n")

    foreach(c11threads_file c11threads.h c11threads_win32.c)
        if(NOT EXISTS "${c11threads_SOURCE_DIR}/${c11threads_file}")
            message(FATAL_ERROR "Fetched c11threads is missing ${c11threads_file}.")
        endif()
    endforeach()

    set(DP_C11_THREADS_REQUIRED_INCLUDES
        "${DP_C11THREADS_GENERATED_INCLUDE_DIR}"
        "${c11threads_SOURCE_DIR}"
    )
    set(DP_C11_THREADS_CHECK_IMPLEMENTATION "#include \"c11threads_win32.c\"")
    set(DP_C11_THREADS_TARGET_INCLUDES ${DP_C11_THREADS_REQUIRED_INCLUDES})
    set(DP_C11_THREADS_TARGET_SOURCES "${c11threads_SOURCE_DIR}/c11threads_win32.c")
else()
    find_package(Threads REQUIRED)
    list(APPEND DP_C11_THREADS_REQUIRED_LIBRARIES Threads::Threads)
endif()

cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_FLAGS "-std=c11")
set(CMAKE_REQUIRED_INCLUDES ${DP_C11_THREADS_REQUIRED_INCLUDES})
set(CMAKE_REQUIRED_LIBRARIES ${DP_C11_THREADS_REQUIRED_LIBRARIES})
check_c_source_compiles("
    #include <threads.h>
    ${DP_C11_THREADS_CHECK_IMPLEMENTATION}

    static int worker(void *arg) {
        (void)arg;
        return 0;
    }

    int main(void) {
        thrd_t thread;
        if(thrd_create(&thread, worker, 0) != thrd_success) {
            return 1;
        }
        thrd_join(thread, 0);
        return 0;
    }
" DP_HAVE_C11_THREADS)
cmake_pop_check_state()

if(NOT DP_HAVE_C11_THREADS)
    message(FATAL_ERROR "DoublePendulum requires C11 <threads.h>. On Windows MinGW, the c11threads fallback was applied but did not compile. On Linux, use a GCC/libc combination with C11 threads support.")
endif()

set(DP_SDL2_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/externals/SDL2-2.32.10")

if(NOT EXISTS "${DP_SDL2_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR "SDL2 submodule is missing. Run: git submodule update --init --recursive")
endif()

set(SDL_SHARED ON CACHE BOOL "Build SDL2 as a shared library" FORCE)
set(SDL_STATIC OFF CACHE BOOL "Do not build SDL2 as a static library" FORCE)
set(SDL_TEST OFF CACHE BOOL "Do not build SDL2_test" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Do not build SDL2 tests" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "Do not install SDL2 from this project" FORCE)
add_subdirectory("${DP_SDL2_ROOT}" "${CMAKE_BINARY_DIR}/externals/SDL2" EXCLUDE_FROM_ALL)

set(DP_APP_SOURCES
    src/main.c
    src/app/app.c
    src/app/fps.c
    src/app/input.c
    src/app/render_data.c
    src/app/window.c
)

set(DP_SIMULATION_SOURCES
    src/simulation/pendulum.c
    src/simulation/pendulum_equations.c
    src/simulation/simulation.c
)

if(DP_INTEGRATOR STREQUAL "rk4")
    set(DP_INTEGRATOR_SOURCES
        src/simulation/integrators/rk4.c
    )
endif()

if(DP_RENDERER STREQUAL "sdl")
    set(DP_RENDERER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/sdl")
    set(DP_RENDERER_SOURCES
        src/renderer/sdl/color.c
        src/renderer/sdl/line_batch.c
        src/renderer/sdl/renderer.c
        src/renderer/sdl/trail.c
    )
endif()

add_executable(DoublePendulum)

target_sources(DoublePendulum PRIVATE
    ${DP_APP_SOURCES}
    ${DP_SIMULATION_SOURCES}
    ${DP_INTEGRATOR_SOURCES}
    ${DP_RENDERER_SOURCES}
    ${DP_C11_THREADS_TARGET_SOURCES}
    src/utils/threadpool.c
)

target_include_directories(DoublePendulum PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/app"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/renderer"
    "${DP_RENDERER_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/simulation"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/simulation/integrators"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils"
)

if(DP_USE_MINGW_C11THREADS)
    target_include_directories(DoublePendulum PRIVATE ${DP_C11_THREADS_TARGET_INCLUDES})
endif()

target_compile_definitions(DoublePendulum PRIVATE
    SDL_MAIN_HANDLED
)

if(DP_INTEGRATOR STREQUAL "rk4")
    target_compile_definitions(DoublePendulum PRIVATE DP_INTEGRATOR_RK4=1)
endif()

if(DP_RENDERER STREQUAL "sdl")
    target_compile_definitions(DoublePendulum PRIVATE DP_RENDERER_SDL=1)
endif()

if(DP_PLATFORM STREQUAL "win32")
    target_compile_definitions(DoublePendulum PRIVATE DP_PLATFORM_WIN32=1)
elseif(DP_PLATFORM STREQUAL "posix")
    target_compile_definitions(DoublePendulum PRIVATE DP_PLATFORM_POSIX=1)
endif()

target_compile_options(DoublePendulum PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(DoublePendulum PRIVATE
    SDL2::SDL2
)

if(TARGET Threads::Threads)
    target_link_libraries(DoublePendulum PRIVATE Threads::Threads)
endif()

if(DP_PLATFORM STREQUAL "posix")
    target_link_libraries(DoublePendulum PRIVATE m)
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" DP_BUILD_TYPE)
if(DP_BUILD_TYPE STREQUAL "RELEASE")
    check_ipo_supported(RESULT DP_IPO_SUPPORTED OUTPUT DP_IPO_ERROR)
    if(DP_IPO_SUPPORTED)
        set_property(TARGET DoublePendulum PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        message(STATUS "Release IPO/LTO enabled.")
    else()
        message(WARNING "Release IPO/LTO is not supported by this toolchain; continuing without LTO. ${DP_IPO_ERROR}")
    endif()
endif()

set_target_properties(DoublePendulum PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${DP_RUNTIME_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${DP_LIBRARY_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${DP_LIBRARY_OUTPUT_DIR}"
)

add_custom_command(TARGET DoublePendulum POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:DoublePendulum>/assets"
    COMMENT "Copying runtime assets"
)
