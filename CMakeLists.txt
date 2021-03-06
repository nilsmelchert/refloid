project(refloid)

# This enforces a particular version of CMake that we require to process the script files
# properly.
cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 11)

# As of CMake 2.6 policies were introduced in order to provide a mechanism for
# adding backwards compatibility one feature at a time.  We will just specify
# that all policies will use version 2.6 symantics.
cmake_policy(VERSION 2.6)

# Locate the NVRT distribution.  Search the SDK first, then look in the system.
set(OptiX_INSTALL_DIR "/home/melchert/Desktop/Projects/raytracing/NVIDIA-OptiX-SDK-6.0.0-linux64" CACHE PATH "Path to OptiX installed location.")


# Add paths to our CMake code to the module path, so they can be found automatically by
# CMake.
set(CMAKE_MODULE_PATH
        "${OptiX_INSTALL_DIR}/SDK/CMake"
        ${CMAKE_MODULE_PATH}
        )

# Set the default build to Release.  Note this doesn't do anything for the VS
# default build target which defaults to Debug when you first start it.
IF (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release" CACHE STRING
            "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
            FORCE)
ENDIF (NOT CMAKE_BUILD_TYPE)

# Tells CMake to build all the libraries as shared libraries by default.  This can be
# overrided by individual libraries later.
set(BUILD_SHARED_LIBS ON)

##########
# Process our custom setup scripts here.

# Include all CMake Macros.
include(Macros)
# Determine information about the compiler
include(CompilerInfo)
# Check for specific machine/compiler options.
include(ConfigCompilerFlags)

# Turn off the warning that NVCC issues when generating PTX from our CUDA samples.  This
# is a custom extension to the FindCUDA code distributed by CMake.
OPTION(CUDA_REMOVE_GLOBAL_MEMORY_SPACE_WARNING "Suppress the \"Advisory: Cannot tell what pointer points to, assuming global memory space\" warning nvcc makes." ON)


# For Xcode 5, gcc is actually clang, so we have to tell CUDA to treat the compiler as
# clang, so that it doesn't mistake it for something else.
if (USING_CLANG_C)
    set(CUDA_HOST_COMPILER "clang" CACHE FILEPATH "Host side compiler used by NVCC")
endif ()

# CUDA 8 is broken for generating dependencies during configure
option(CUDA_GENERATE_DEPENDENCIES_DURING_CONFIGURE "Generate dependencies during configure time instead of only during build time." OFF)

# Find at least a 5.0 version of CUDA.
find_package(CUDA 5.0 REQUIRED)

# Present the CUDA_64_BIT_DEVICE_CODE on the default set of options.
mark_as_advanced(CLEAR CUDA_64_BIT_DEVICE_CODE)


# Add some useful default arguments to the NVCC and NVRTC flags.  This is an example of
# how we use PASSED_FIRST_CONFIGURE.  Once you have configured, this variable is TRUE
# and following block of code will not be executed leaving you free to edit the values
# as much as you wish from the GUI or from ccmake.
if (NOT PASSED_FIRST_CONFIGURE)
    list(FIND CUDA_NVCC_FLAGS "-arch" index)
    if (index EQUAL -1)
        list(APPEND CUDA_NVCC_FLAGS -arch sm_30)
        set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} CACHE LIST "Semi-colon delimit multiple arguments." FORCE)
    endif ()
    set(flag "--use_fast_math")
    list(FIND CUDA_NVCC_FLAGS ${flag} index)
    if (index EQUAL -1)
        list(APPEND CUDA_NVCC_FLAGS ${flag})
        set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} CACHE LIST "Semi-colon delimit multiple arguments." FORCE)
    endif ()

    if (CUDA_VERSION VERSION_LESS "3.0")
        set(flag "--keep")
        list(FIND CUDA_NVCC_FLAGS ${flag} index)
        if (index EQUAL -1)
            list(APPEND CUDA_NVCC_FLAGS ${flag})
            set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} CACHE LIST "Semi-colon delimit multiple arguments." FORCE)
        endif ()
    endif ()

    set(CUDA_NVRTC_FLAGS -arch compute_30 -use_fast_math -lineinfo -default-device -rdc true -D__x86_64 CACHE LIST "Semi-colon delimit multiple arguments." FORCE)
endif (NOT PASSED_FIRST_CONFIGURE)

# Add required nvcc flag for callable programs under CUDA 8
if (CUDA_VERSION VERSION_GREATER "7.5")
    set(flag "--keep-device-functions")
    list(FIND CUDA_NVCC_FLAGS ${flag} index)
    if (index EQUAL -1)
        list(APPEND CUDA_NVCC_FLAGS ${flag})
        set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} CACHE LIST "Semi-colon delimit multiple arguments." FORCE)
    endif ()
endif ()

# This passes a preprocessor definition to cl.exe when processing CUDA code.
if (USING_WINDOWS_CL)
    list(APPEND CUDA_NVCC_FLAGS --compiler-options /D_USE_MATH_DEFINES)
endif ()

mark_as_advanced(CUDA_NVRTC_FLAGS)

# Put all the runtime stuff in the same directory.  By default, CMake puts each targets'
# output into their own directory.  We want all the targets to be put in the same
# directory, and we can do this by setting these variables.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
add_definitions( -DOPTIX_BIN_PATH=\"${CMAKE_BINARY_DIR}/lib/ptx/\")
add_definitions( -DOPTIX_BASE_PATH=\"${OptiX_INSTALL_DIR}/\")

# Create a flag for mac which will allow apps to add the local cuda toolkit
# install path to the app's rpath.
if (APPLE)
    set(CUDA_TOOLKIT_RPATH_FLAG "-Wl,-rpath,${CUDA_TOOLKIT_ROOT_DIR}/lib")
endif ()

# Search for the OptiX libraries and include files.
find_package(OptiX REQUIRED)

## load in pkg-config support
find_package(PkgConfig)
find_package(TIFF)

# Optional: When IL_FOUND is false after this call, the OptiX introduction samples optixIntro_07 and higher will not be built.
# find_package(DevIL)

## use pkg-config to get hints for 0mq locations
pkg_check_modules(PC_ZeroMQ QUIET zmq)

# Search for zeromq
find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

## use the hint from about to find the location of libzmq

find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )

# Add the path to the OptiX headers to our include paths.
include_directories(
        "${OptiX_INCLUDE}"
        "${CMAKE_SOURCE_DIR}/external_libs/spdlog"
        "${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/src/host"
        "${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME}/src/device"
        "${OptiX_INSTALL_DIR}/SDK/cuda"
)
include_directories(
        "${TIFF_INCLUDE_DIRS}"
        )

# Select whether to use NVRTC or NVCC to generate PTX
set(CUDA_NVRTC_ENABLED ON CACHE BOOL "Use NVRTC to compile PTX at run-time instead of NVCC at build-time")

##################################################################
# SUtil compilation

set(OPTIX_SAMPLES_SDK_PTX_DIR "${CMAKE_BINARY_DIR}/lib/ptx")
set(SAMPLES_PTX_DIR "${CMAKE_BINARY_DIR}/lib/ptx")
# set(SAMPLES_DIR "${OptiX_INSTALL_DIR}/SDK")
set(SAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CUDA_GENERATED_OUTPUT_DIR ${SAMPLES_PTX_DIR})

if (WIN32)
    string(REPLACE "/" "\\\\" SAMPLES_PTX_DIR ${SAMPLES_PTX_DIR})
else (WIN32)
    if (USING_GNU_C AND NOT APPLE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DM_PI=3.14159265358979323846")
    endif ()
endif (WIN32)

# NVRTC include paths relative to the sample path
set(SAMPLES_RELATIVE_INCLUDE_DIRS "\\
  \"/sutil\", \\
  \"/cuda\", ")

# NVRTC absolute include paths to the headers used to build the samples
set(SAMPLES_ABSOLUTE_INCLUDE_DIRS "\\
  \"${OptiX_INCLUDE}\", \\
  \"${OptiX_INCLUDE}/optixu\", \\
  \"${OptiX_INSTALL_DIR}/SDK/support/mdl-sdk/include\", \\
  \"${CUDA_INCLUDE_DIRS}\", ")

# Build a null-terminated option list for NVRTC
set(CUDA_NVRTC_OPTIONS)
foreach (flag ${CUDA_NVRTC_FLAGS})
    set(CUDA_NVRTC_OPTIONS "${CUDA_NVRTC_OPTIONS} \\\n  \"${flag}\",")
endforeach ()
set(CUDA_NVRTC_OPTIONS "${CUDA_NVRTC_OPTIONS} \\\n  0,")

configure_file(${OptiX_INSTALL_DIR}/SDK/sampleConfig.h.in ${CMAKE_BINARY_DIR}/sampleConfig.h @ONLY)

# Path to sutil.h that all the samples need
include_directories(${OptiX_INSTALL_DIR}/SDK/sutil
        ${OptiX_INSTALL_DIR}/SDK
        ${OptiX_INCLUDE}/optixu
        ${OptiX_INSTALL_DIR}/SDK/support/mdl-sdk/include
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CUDA_INCLUDE_DIRS})

include(FindSUtilGLUT)


set(SAMPLES_INCLUDE_DIR ${OptiX_INSTALL_DIR}/SDK/cuda)
if (EXISTS ${OptiX_INSTALL_DIR}/SDK/support/mdl-sdk/include/mi/mdl_sdk.h)
    # The MDL_SDK wrapper library can only be built when the MDL SDK is available
    add_subdirectory(mdl_wrapper)
    set(USE_OWN_MDL_WRAPPER TRUE)
else ()
    include(FindMDLWrapper)
endif ()

set(SAMPLES_CUDA_DIR ${OptiX_INSTALL_DIR}/SDK/cuda)

#########################################################
# OPTIX_add_sample_executable
#
# Convience function for adding samples to the code.  You can copy the contents of this
# funtion into your individual project if you wish to customize the behavior.  Note that
# in CMake, functions have their own scope, whereas macros use the scope of the caller.
function(OPTIX_add_sample_executable target_name)

    # These calls will group PTX and CUDA files into their own directories in the Visual
    # Studio projects.
    source_group("PTX Files" REGULAR_EXPRESSION ".+\\.ptx$")
    source_group("CUDA Files" REGULAR_EXPRESSION ".+\\.cu$")

    # Separate the sources from the CMake and CUDA options fed to the macro.  This code
    # comes from the CUDA_COMPILE_PTX macro found in FindCUDA.cmake.  We are copying the
    # code here, so that we can use our own name for the target.  target_name is used in the
    # creation of the output file names, and we want this to be unique for each target in
    # the SDK.
    CUDA_GET_SOURCES_AND_OPTIONS(source_files cmake_options options ${ARGN})

    # Create the rules to build the PTX from the CUDA files.
    CUDA_WRAP_SRCS(${target_name} PTX generated_files ${source_files} ${cmake_options}
            OPTIONS ${options})


    # Find includes in corresponding build directories
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    # Instruct CMake to run moc automatically when needed.
    set(CMAKE_AUTOMOC ON)

    # Find the QtWidgets library
    find_package(Qt5Widgets)

    # Here is where we create the rule to make the executable.  We define a target name and
    # list all the source files used to create the target.  In addition we also pass along
    # the cmake_options parsed out of the arguments.
    add_executable(${target_name}
            ${source_files}
            ${generated_files}
            ${cmake_options}
            )


    # The Qt5Widgets_INCLUDES also includes the include directories for
    # dependencies QtCore and QtGui
    include_directories(${Qt5Widgets_INCLUDES})

    # We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
    add_definitions(${Qt5Widgets_DEFINITIONS})

    # Executables fail to build with Qt 5 in the default configuration
    # without -fPIE. We add that here.
    set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

    # Most of the samples link against sutil, optix, and glfw.  Here is the
    # rule that specifies this linkage.
    #            imgui
    #glfw

    target_link_libraries(${target_name}
            ${TIFF_LIBRARIES}
            Qt5::Widgets
            sutil_sdk
            optix
            ${ZeroMQ_LIBRARY}
            ${optix_rpath}
            )
    if (USING_GNU_CXX)
        target_link_libraries(${target_name} m) # Explicitly link against math library (C samples don't do that by default)
    endif ()
endfunction()

#########################################################
#  List of samples found in subdirectories.
#
# If you wish to start your own sample, you can copy one of the sample's directories.
# Just make sure you rename all the occurances of the sample's name in the C code as well
# and the CMakeLists.txt file.
# add_subdirectory(optixBuffersOfBuffers)
add_subdirectory(refloid)

# Our sutil library.  The rules to build it are found in the subdirectory.
# add_subdirectory(sutil)
 add_subdirectory(${OptiX_INSTALL_DIR}/SDK/sutil ${CMAKE_BINARY_DIR}/sutil)
#add_subdirectory(${OptiX_INSTALL_DIR}/SDK/sutil)

# This copies out dlls into the build directories, so that users no longer need to copy
# them over in order to run the samples.  This depends on the optixHello sample being compiled.
# If you remove this sample from the list of compiled samples, then you should change
# "optixHello" found below to the name of one of your other samples.
if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT APPLE)
        set(bit_dest "64")
    else ()
        set(bit_dest "")
    endif ()
    foreach (config ${CMAKE_CONFIGURATION_TYPES})
        cmake_policy(SET CMP0026 OLD)  # disable warning about LOCATION property
        get_target_property(loc optixHello ${config}_LOCATION)
        if (loc)
            # A little helper function
            function(copy_dll lib)
                get_filename_component(path ${loc} PATH)
                get_filename_component(name ${lib} NAME)
                execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${lib} ${path}/${name})
            endfunction()

            # Copy the binary directory into the build directory
            file(GLOB dlls "${OptiX_INSTALL_DIR}/bin${bit_dest}/*.dll")
            foreach (file ${dlls})
                copy_dll("${file}")
            endforeach ()

            # Copy the shipped MDL DLLs into the build directory if we didn't build our own
            if (NOT USE_OWN_MDL_WRAPPER)
                file(GLOB dlls "${OptiX_INSTALL_DIR}/SDK/support/mdl_wrapper/lib/*.dll")
                foreach (file ${dlls})
                    copy_dll("${file}")
                endforeach ()
            endif ()
        else ()
            message(WARNING "Unable to find location to copy DLLs into the build")
        endif ()
    endforeach ()
endif (WIN32)

#################################################################

# Now that everything is done, indicate that we have finished configuring at least once.
# We use this variable to set certain defaults only on the first pass, so that we don't
# continually set them over and over again.
set(PASSED_FIRST_CONFIGURE ON CACHE INTERNAL "Already Configured once?")
