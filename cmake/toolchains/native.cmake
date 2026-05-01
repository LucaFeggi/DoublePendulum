include_guard(GLOBAL)

function(double_pendulum_pick_latest_path out_var)
    set(_paths ${ARGN})
    if(NOT _paths)
        set(${out_var} "" PARENT_SCOPE)
        return()
    endif()

    list(SORT _paths COMPARE NATURAL ORDER DESCENDING)
    list(GET _paths 0 _latest_path)
    set(${out_var} "${_latest_path}" PARENT_SCOPE)
endfunction()

function(double_pendulum_find_msvc_tools out_var)
    if(DEFINED ENV{VCToolsInstallDir} AND EXISTS "$ENV{VCToolsInstallDir}/bin/Hostx64/x64/cl.exe")
        set(${out_var} "$ENV{VCToolsInstallDir}" PARENT_SCOPE)
        return()
    endif()

    if(DEFINED ENV{VCINSTALLDIR})
        file(GLOB _vc_tools LIST_DIRECTORIES true "$ENV{VCINSTALLDIR}/Tools/MSVC/*")
        double_pendulum_pick_latest_path(_vc_tools_dir ${_vc_tools})
        if(_vc_tools_dir AND EXISTS "${_vc_tools_dir}/bin/Hostx64/x64/cl.exe")
            set(${out_var} "${_vc_tools_dir}" PARENT_SCOPE)
            return()
        endif()
    endif()

    set(_vswhere "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer/vswhere.exe")
    if(EXISTS "${_vswhere}")
        execute_process(
            COMMAND "${_vswhere}" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
            OUTPUT_VARIABLE _vs_install_dir
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )

        if(_vs_install_dir)
            file(GLOB _vc_tools LIST_DIRECTORIES true "${_vs_install_dir}/VC/Tools/MSVC/*")
            double_pendulum_pick_latest_path(_vc_tools_dir ${_vc_tools})
            if(_vc_tools_dir AND EXISTS "${_vc_tools_dir}/bin/Hostx64/x64/cl.exe")
                set(${out_var} "${_vc_tools_dir}" PARENT_SCOPE)
                return()
            endif()
        endif()
    endif()

    set(${out_var} "" PARENT_SCOPE)
endfunction()

function(double_pendulum_find_windows_sdk_tool out_var tool_name)
    if(DEFINED ENV{WindowsSdkDir} AND DEFINED ENV{WindowsSDKVersion})
        set(_sdk_tool "$ENV{WindowsSdkDir}/bin/$ENV{WindowsSDKVersion}/x64/${tool_name}.exe")
        if(EXISTS "${_sdk_tool}")
            set(${out_var} "${_sdk_tool}" PARENT_SCOPE)
            return()
        endif()
    endif()

    file(GLOB _sdk_tools LIST_DIRECTORIES false "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/bin/*/x64/${tool_name}.exe")
    double_pendulum_pick_latest_path(_sdk_tool ${_sdk_tools})
    set(${out_var} "${_sdk_tool}" PARENT_SCOPE)
endfunction()

function(double_pendulum_find_windows_sdk_root out_var)
    if(DEFINED ENV{WindowsSdkDir} AND EXISTS "$ENV{WindowsSdkDir}/Include")
        file(TO_CMAKE_PATH "$ENV{WindowsSdkDir}" _sdk_root)
        set(${out_var} "${_sdk_root}" PARENT_SCOPE)
        return()
    endif()

    if(EXISTS "$ENV{ProgramFiles\(x86\)}/Windows Kits/10")
        set(${out_var} "$ENV{ProgramFiles\(x86\)}/Windows Kits/10" PARENT_SCOPE)
        return()
    endif()

    set(${out_var} "" PARENT_SCOPE)
endfunction()

function(double_pendulum_find_windows_sdk_version out_var sdk_root)
    if(DEFINED ENV{WindowsSDKVersion} AND EXISTS "${sdk_root}/Include/$ENV{WindowsSDKVersion}/ucrt")
        set(${out_var} "$ENV{WindowsSDKVersion}" PARENT_SCOPE)
        return()
    endif()

    file(GLOB _sdk_include_versions LIST_DIRECTORIES true "${sdk_root}/Include/*")
    double_pendulum_pick_latest_path(_sdk_include_dir ${_sdk_include_versions})
    if(_sdk_include_dir)
        get_filename_component(_sdk_version "${_sdk_include_dir}" NAME)
        set(${out_var} "${_sdk_version}" PARENT_SCOPE)
        return()
    endif()

    set(${out_var} "" PARENT_SCOPE)
endfunction()

function(double_pendulum_prepend_env_path name)
    set(_paths ${ARGN})
    list(FILTER _paths EXCLUDE REGEX "^$")

    if(DEFINED ENV{${name}} AND NOT "$ENV{${name}}" STREQUAL "")
        list(APPEND _paths "$ENV{${name}}")
    endif()

    string(JOIN ";" _joined_paths ${_paths})
    set(ENV{${name}} "${_joined_paths}")
endfunction()

function(double_pendulum_set_msvc_x64_environment msvc_tools_dir msvc_bin_dir sdk_root sdk_version)
    set(_include_paths
        "${msvc_tools_dir}/include"
        "${sdk_root}/Include/${sdk_version}/ucrt"
        "${sdk_root}/Include/${sdk_version}/shared"
        "${sdk_root}/Include/${sdk_version}/um"
        "${sdk_root}/Include/${sdk_version}/winrt"
        "${sdk_root}/Include/${sdk_version}/cppwinrt"
    )

    if(EXISTS "${msvc_tools_dir}/atlmfc/include")
        list(APPEND _include_paths "${msvc_tools_dir}/atlmfc/include")
    endif()

    set(_lib_paths
        "${msvc_tools_dir}/lib/x64"
        "${sdk_root}/Lib/${sdk_version}/ucrt/x64"
        "${sdk_root}/Lib/${sdk_version}/um/x64"
    )

    if(EXISTS "${msvc_tools_dir}/atlmfc/lib/x64")
        list(APPEND _lib_paths "${msvc_tools_dir}/atlmfc/lib/x64")
    endif()

    set(_libpath_paths
        "${msvc_tools_dir}/lib/x64"
        "${sdk_root}/UnionMetadata/${sdk_version}"
        "${sdk_root}/References/${sdk_version}"
    )

    if(EXISTS "${msvc_tools_dir}/atlmfc/lib/x64")
        list(APPEND _libpath_paths "${msvc_tools_dir}/atlmfc/lib/x64")
    endif()

    set(_path_paths
        "${msvc_bin_dir}"
        "${sdk_root}/bin/${sdk_version}/x64"
    )

    string(JOIN ";" _include_value ${_include_paths})
    string(JOIN ";" _lib_value ${_lib_paths})
    string(JOIN ";" _libpath_value ${_libpath_paths})
    set(ENV{INCLUDE} "${_include_value}")
    set(ENV{LIB} "${_lib_value}")
    set(ENV{LIBPATH} "${_libpath_value}")

    double_pendulum_prepend_env_path(PATH ${_path_paths})
endfunction()

function(double_pendulum_set_msvc_x64_link_flags msvc_tools_dir sdk_root sdk_version)
    set(_link_dirs
        "${msvc_tools_dir}/lib/x64"
        "${sdk_root}/Lib/${sdk_version}/ucrt/x64"
        "${sdk_root}/Lib/${sdk_version}/um/x64"
    )

    if(EXISTS "${msvc_tools_dir}/atlmfc/lib/x64")
        list(APPEND _link_dirs "${msvc_tools_dir}/atlmfc/lib/x64")
    endif()

    set(_link_flags "/machine:x64")
    foreach(_link_dir IN LISTS _link_dirs)
        string(APPEND _link_flags " /LIBPATH:\"${_link_dir}\"")
    endforeach()

    set(CMAKE_EXE_LINKER_FLAGS "${_link_flags}" CACHE STRING "x64 MSVC executable linker flags" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS "${_link_flags}" CACHE STRING "x64 MSVC shared library linker flags" FORCE)
    set(CMAKE_MODULE_LINKER_FLAGS "${_link_flags}" CACHE STRING "x64 MSVC module linker flags" FORCE)
endfunction()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    double_pendulum_find_msvc_tools(DOUBLE_PENDULUM_MSVC_TOOLS_DIR)
    if(NOT DOUBLE_PENDULUM_MSVC_TOOLS_DIR)
        message(FATAL_ERROR "Could not find the MSVC x64 toolchain. Install Visual Studio C++ tools or run from a Visual Studio Developer shell.")
    endif()

    set(DOUBLE_PENDULUM_MSVC_BIN_DIR "${DOUBLE_PENDULUM_MSVC_TOOLS_DIR}/bin/Hostx64/x64")

    double_pendulum_find_windows_sdk_root(DOUBLE_PENDULUM_WINDOWS_SDK_ROOT)
    if(NOT DOUBLE_PENDULUM_WINDOWS_SDK_ROOT)
        message(FATAL_ERROR "Could not find the Windows 10 SDK.")
    endif()

    double_pendulum_find_windows_sdk_version(DOUBLE_PENDULUM_WINDOWS_SDK_VERSION "${DOUBLE_PENDULUM_WINDOWS_SDK_ROOT}")
    if(NOT DOUBLE_PENDULUM_WINDOWS_SDK_VERSION)
        message(FATAL_ERROR "Could not find a Windows 10 SDK version.")
    endif()

    double_pendulum_set_msvc_x64_environment(
        "${DOUBLE_PENDULUM_MSVC_TOOLS_DIR}"
        "${DOUBLE_PENDULUM_MSVC_BIN_DIR}"
        "${DOUBLE_PENDULUM_WINDOWS_SDK_ROOT}"
        "${DOUBLE_PENDULUM_WINDOWS_SDK_VERSION}"
    )

    double_pendulum_set_msvc_x64_link_flags(
        "${DOUBLE_PENDULUM_MSVC_TOOLS_DIR}"
        "${DOUBLE_PENDULUM_WINDOWS_SDK_ROOT}"
        "${DOUBLE_PENDULUM_WINDOWS_SDK_VERSION}"
    )

    set(CMAKE_C_COMPILER "${DOUBLE_PENDULUM_MSVC_BIN_DIR}/cl.exe" CACHE FILEPATH "C compiler for native Windows builds" FORCE)
    set(CMAKE_LINKER "${DOUBLE_PENDULUM_MSVC_BIN_DIR}/link.exe" CACHE FILEPATH "Linker for native Windows builds" FORCE)

    double_pendulum_find_windows_sdk_tool(DOUBLE_PENDULUM_RC_COMPILER rc)
    if(DOUBLE_PENDULUM_RC_COMPILER)
        set(CMAKE_RC_COMPILER "${DOUBLE_PENDULUM_RC_COMPILER}" CACHE FILEPATH "Resource compiler for native Windows builds" FORCE)
    endif()

    double_pendulum_find_windows_sdk_tool(DOUBLE_PENDULUM_MT mt)
    if(DOUBLE_PENDULUM_MT)
        set(CMAKE_MT "${DOUBLE_PENDULUM_MT}" CACHE FILEPATH "Manifest tool for native Windows builds" FORCE)
    endif()

    if(EXISTS "${DOUBLE_PENDULUM_MSVC_TOOLS_DIR}/bin/Hostx64/x64/ninja.exe")
        set(CMAKE_MAKE_PROGRAM "${DOUBLE_PENDULUM_MSVC_TOOLS_DIR}/bin/Hostx64/x64/ninja.exe" CACHE FILEPATH "Ninja for native Windows builds" FORCE)
    elseif(DEFINED ENV{VCINSTALLDIR} AND EXISTS "$ENV{VCINSTALLDIR}/Tools/Llvm/x64/bin/ninja.exe")
        set(CMAKE_MAKE_PROGRAM "$ENV{VCINSTALLDIR}/Tools/Llvm/x64/bin/ninja.exe" CACHE FILEPATH "Ninja for native Windows builds" FORCE)
    endif()
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_C_COMPILER "gcc" CACHE STRING "C compiler for native Linux builds" FORCE)
else()
    message(FATAL_ERROR "Unsupported host system: ${CMAKE_HOST_SYSTEM_NAME}")
endif()
