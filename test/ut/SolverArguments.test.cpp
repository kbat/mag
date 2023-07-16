#include <gtest/gtest.h>
#include <SolverArguments.h>

TEST(SolverArguments_Test, CheckConstructor) {
    int argc = 4;
    const char* argv[4] = {"mag-solve", "-test", "1", "1"};

    auto args = SolverArguments(argc, argv);
    EXPECT_TRUE(args.IsTest());
    EXPECT_FALSE(args.IsMaterials());
    EXPECT_FALSE(args.IsHelp());
    auto testArgs = args.GetMap()["test"].as<std::vector<size_t> >();
    EXPECT_EQ(testArgs[0], 1);
    EXPECT_EQ(testArgs[1], 1);
}
