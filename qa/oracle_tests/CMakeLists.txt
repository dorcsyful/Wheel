# Demonstrates the oracle library (qa/oracles) against real engine scenes:
# each oracle is proven to pass on correct behavior and, where cheap, to
# actually catch a deliberately wrong value. Kept as its own binary for the
# same reason as harness_tests: engine-mechanics tests, harness-
# infrastructure tests, and oracle-library tests are three different layers.
#
# Reuses the gtest/gtest_main targets defined by qa/Google_tests/lib — this
# subdirectory must be added after qa/Google_tests in the root CMakeLists.

add_executable(QA_Oracle_Tests
        Analytical.cpp
        Invariants.cpp
        Metamorphic.cpp
        GoldenMaster.cpp)
target_link_libraries(QA_Oracle_Tests gtest gtest_main wheel_engine qa_harness qa_oracles)

# So golden-master tests can locate qa/oracle_tests/golden/ regardless of the
# process's working directory (IDE, CTest, and CLI invocations all differ).
target_compile_definitions(QA_Oracle_Tests PRIVATE
        QA_ORACLE_TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}")