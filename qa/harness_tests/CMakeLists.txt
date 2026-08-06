# Tests for the QA harness infrastructure itself (HeadlessSim, and later the
# oracle library / coverage-mutation glue). Kept as its own binary, separate
# from Google_Tests_run, so engine-mechanics unit tests and harness-
# infrastructure tests never share a target.
#
# Reuses the gtest/gtest_main targets defined by qa/Google_tests/lib — this
# subdirectory must be added after qa/Google_tests in the root CMakeLists.

add_executable(QA_Harness_Tests HeadlessSim.cpp)
target_link_libraries(QA_Harness_Tests gtest gtest_main wheel_engine qa_harness)