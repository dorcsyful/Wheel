# Headless simulation entry points and the oracle
# library, coverage/mutation driver glue, and agent tool interface. Depends
# on wheel_engine

file(GLOB_RECURSE QA_HARNESS_SOURCES CONFIGURE_DEPENDS "harness/*.cpp" "harness/*.h")
add_library(qa_harness STATIC ${QA_HARNESS_SOURCES})

target_link_libraries(qa_harness PUBLIC wheel_engine)
target_include_directories(qa_harness PUBLIC harness/)

# Hand-written oracle library (step 2): analytical/invariant/metamorphic/
# golden-master assertions. Framework-agnostic (no gtest dependency) so the
# same OracleResult shape works from hand-written tests now and the agent
# loop later.
file(GLOB_RECURSE QA_ORACLES_SOURCES CONFIGURE_DEPENDS "oracles/*.cpp" "oracles/*.h")
add_library(qa_oracles STATIC ${QA_ORACLES_SOURCES})

target_link_libraries(qa_oracles PUBLIC qa_harness)
target_include_directories(qa_oracles PUBLIC oracles/)