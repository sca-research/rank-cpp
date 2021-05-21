# Tester executables

# Catch2
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.6
)
FetchContent_MakeAvailable(Catch2)

# Unit tests in a single executable
add_executable(tester
  "${CMAKE_CURRENT_SOURCE_DIR}/UnitTests.cpp"

  "${CMAKE_CURRENT_SOURCE_DIR}/BitSpanTests.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/DimensionsTests.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/KeyTests.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/RankTests.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ScoresTableTests.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/WeightTableTests.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/utils/EncodingTests.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/utils/NumericTests.cpp"
)
target_link_libraries(tester PRIVATE
  project_warnings
  project_options
  rankcpp
  Catch2
  GSL
  range-v3
)
target_compile_features(tester PUBLIC cxx_std_17)
set_target_properties(tester PROPERTIES CXX_EXTENSIONS OFF)


