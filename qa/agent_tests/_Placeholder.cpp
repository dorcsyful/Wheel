#include <gtest/gtest.h>

// Keeps the glob in CMakeLists.txt non-empty before the agent has written
// anything. Safe to delete once real agent-authored tests exist alongside it.
TEST(AgentTestsPlaceholder, InterfaceIsWired)
{
    SUCCEED();
}
