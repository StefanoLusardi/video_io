#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    // Run only video_reader tests
    ::testing::GTEST_FLAG(filter) = "video_reader_test.*";
    
    // Run only video_writer tests
    // ::testing::GTEST_FLAG(filter) = "video_writer_test.*";

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
