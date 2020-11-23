# Tester executables
# Catch2
set (CATCH_INCLUDE_DIR
    "${PROJECT_SOURCE_DIR}/third_party/Catch2/single_include")
add_library(Catch2::Catch2 IMPORTED INTERFACE)
set_property(TARGET Catch2::Catch2
    PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CATCH_INCLUDE_DIR}")

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
    Catch2::Catch2
    GSL
    RangeV3
)
target_compile_features(tester PUBLIC cxx_std_17)
set_target_properties(tester PROPERTIES CXX_EXTENSIONS OFF)


