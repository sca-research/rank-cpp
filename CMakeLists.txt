cmake_minimum_required (VERSION 3.14...3.18)

# Workaround to allow us to support the new VERSION syntax introduced > 3.12
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project (rankcpp VERSION 1.0
                 DESCRIPTION "side-channel key rank and enumeration routines"
                 LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(FetchContent)

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

# enable Sphinx
include(cmake/FindSphinx.cmake)

# GSL (Microsoft guidelines support library)
FetchContent_Declare(
  GSL
  GIT_REPOSITORY https://github.com/microsoft/gsl.git
  GIT_TAG        v3.1.0
)
FetchContent_MakeAvailable(GSL)

# range-v3
FetchContent_Declare(
  range-v3
  GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
  GIT_TAG        e7746b214f3cfca341a8aa55fbce95b6b8a1c9d1
)
FetchContent_MakeAvailable(range-v3)

# Main rank-cpp library (header-only)
add_library(rankcpp INTERFACE)
target_include_directories(rankcpp INTERFACE "${PROJECT_SOURCE_DIR}/include/")
target_compile_features(rankcpp INTERFACE cxx_std_17)
target_link_libraries(rankcpp INTERFACE GSL range-v3)

# Test binaries
add_subdirectory("test")

# Documentation
if(ENABLE_DOC)
  add_subdirectory ("doc")
endif()
