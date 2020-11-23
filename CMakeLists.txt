cmake_minimum_required (VERSION 3.9...3.18)

# Workaround to allow us to support the new VERSION syntax introduced > 3.12
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project (rankcpp VERSION 1.0
                 DESCRIPTION "side-channel key rank and enumeration routines"
                 LANGUAGES CXX)

# common cmake
include(cmake/StdProjectSettings.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

# Clang: option to enable -ftime-trace
if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE
    "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    add_compile_definitions(project_options INTERFACE -ftime-trace)
  endif()
endif()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# enable doxygen
include(cmake/Doxygen.cmake)
enable_doxygen()

# Main baikal library (header-only)
add_library(rankcpp INTERFACE)
target_include_directories(rankcpp INTERFACE "${PROJECT_SOURCE_DIR}/include/")
target_compile_features(rankcpp INTERFACE cxx_std_17)

# GSL (Microsoft guidelines support library)
set (GSL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/third_party/gsl/include/")
add_library(GSL IMPORTED INTERFACE)
set_property(TARGET GSL
    PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${GSL_INCLUDE_DIR}")

# range-v3
set (RANGEV3_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/third_party/range-v3/include/")
add_library(RangeV3 IMPORTED INTERFACE)
set_property(TARGET RangeV3
    PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${RANGEV3_INCLUDE_DIR}")

# Test binaries
add_subdirectory("test")
