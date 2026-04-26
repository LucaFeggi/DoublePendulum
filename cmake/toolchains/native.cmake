include_guard(GLOBAL)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_C_COMPILER "cl" CACHE STRING "C compiler for native Windows builds" FORCE)
    set(CMAKE_LINKER "link" CACHE STRING "Linker for native Windows builds" FORCE)

    if(DEFINED ENV{VCINSTALLDIR} AND EXISTS "$ENV{VCINSTALLDIR}/Tools/Llvm/x64/bin/ninja.exe")
        set(CMAKE_MAKE_PROGRAM "$ENV{VCINSTALLDIR}/Tools/Llvm/x64/bin/ninja.exe" CACHE FILEPATH "Ninja for native Windows builds" FORCE)
    endif()
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_C_COMPILER "gcc" CACHE STRING "C compiler for native Linux builds" FORCE)
else()
    message(FATAL_ERROR "Unsupported host system: ${CMAKE_HOST_SYSTEM_NAME}")
endif()
