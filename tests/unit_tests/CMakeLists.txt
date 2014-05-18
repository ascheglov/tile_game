add_executable(unit_tests
    ../common/test_printers.hpp
    TestCanvas.hpp
    math_tests.cpp
    unit_tests.cpp)

add_custom_command(
  TARGET unit_tests POST_BUILD
  COMMAND unit_tests
  COMMENT "Running unit tests..."
)
