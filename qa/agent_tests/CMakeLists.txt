# Where the agent's generated tests land. Auto-discovers .cpp files via glob
# -- unlike the hand-written suites' curated explicit lists, this directory
# gets new files written to it programmatically (qa/agent_interface/
# submit_test.py), so there's no human editing this file per submission.
#
# _Placeholder.cpp keeps the glob non-empty before the agent has written
# anything; delete it once real agent-authored tests exist here.

file(GLOB AGENT_TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(Agent_Tests ${AGENT_TEST_SOURCES})
target_link_libraries(Agent_Tests gtest gtest_main wheel_engine qa_harness qa_oracles)