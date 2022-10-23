#include "test_video_writer.hpp"

namespace vc::test
{

TEST_F(video_writer_test, open_without_duration)
{
    // use filesystem path instead of string.
    const auto video_path = test_data_directory + test_name + ".mp4";
    ASSERT_TRUE(v->open(video_path, width, height, fps));
}

TEST_F(video_writer_test, open_with_duration)
{
    const auto video_path = test_data_directory + "out.mp4";
    ASSERT_TRUE(v->open(video_path, width, height, fps, duration));
}

TEST_F(video_writer_test, open_invalid_width)
{
    const auto video_path = test_data_directory + "out.mp4";
    ASSERT_FALSE(v->open(video_path, -123, height, fps));
}

TEST_F(video_writer_test, open_invalid_height)
{
    const auto video_path = test_data_directory + "out.mp4";
    ASSERT_FALSE(v->open(video_path, width, -123, fps));
}

TEST_F(video_writer_test, open_invalid_fps)
{
    const auto video_path = test_data_directory + "out.mp4";
    ASSERT_FALSE(v->open(video_path, width, height, -123));
}

TEST_F(video_writer_test, open_invalid_duration)
{
    const auto video_path = test_data_directory + "out.mp4";
    ASSERT_FALSE(v->open(video_path, width, height, fps, -123));
}

TEST_F(video_writer_test, open_invalid_extension_default_to_mpeg)
{
    const auto video_path = test_data_directory + "out.invalid-extension";
    ASSERT_TRUE(v->open(video_path, width, height, fps));
    
    ASSERT_TRUE(v->write(frame_data.data()));
    ASSERT_TRUE(v->save());

    // verify written file is mpeg format
}

TEST_F(video_writer_test, open_release_without_write)
{
    const auto video_path = test_data_directory + "out.mp4";

    ASSERT_TRUE(v->open(video_path, width, height, fps));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->release());
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_writer_test, open_save_without_write)
{
    const auto video_path = test_data_directory + "out.mp4";

    ASSERT_TRUE(v->open(video_path, width, height, fps));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->save());
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_writer_test, open_write_release_without_save)
{
    const auto video_path = test_data_directory + "out.mp4";

    ASSERT_TRUE(v->open(video_path, width, height, fps));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->write(frame_data.data()));

    const auto is_released = v->release();
    ASSERT_TRUE(is_released);
    ASSERT_FALSE(v->is_opened());

    // assert file does not exists
}

TEST_F(video_writer_test, open_write_save_without_release)
{
    const auto video_path = test_data_directory + "out.mp4";

    ASSERT_TRUE(v->open(video_path, width, height, fps));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->write(frame_data.data()));

    ASSERT_TRUE(v->save());
    ASSERT_FALSE(v->is_opened());

    // assert file exists
}

TEST_F(video_writer_test, open_write_save_release)
{
    const auto video_path = test_data_directory + "out.mp4";

    ASSERT_TRUE(v->open(video_path, width, height, fps));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->write(frame_data.data()));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->save());
    ASSERT_FALSE(v->is_opened());

    ASSERT_FALSE(v->release());
    ASSERT_FALSE(v->is_opened());

    // assert file exists
}

TEST_F(video_writer_test, write_without_open)
{
    ASSERT_FALSE(v->is_opened());
    ASSERT_FALSE(v->write(frame_data.data()));

    ASSERT_FALSE(v->is_opened());
    ASSERT_FALSE(v->release());
}

TEST_F(video_writer_test, save_without_open)
{
    ASSERT_FALSE(v->is_opened());
    ASSERT_FALSE(v->save());
    
    ASSERT_FALSE(v->is_opened());
    ASSERT_FALSE(v->release());
}

TEST_F(video_writer_test, release_without_open)
{
    ASSERT_FALSE(v->is_opened());
    ASSERT_FALSE(v->release());
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_writer_test, open_same_path_three_times)
{
    const auto video_path = test_data_directory + "out.mp4";
    ASSERT_TRUE(v->open(video_path, width, height, fps));
    ASSERT_TRUE(v->open(video_path, width, height, fps));
    ASSERT_TRUE(v->open(video_path, width, height, fps));

    ASSERT_TRUE(v->is_opened());
}

TEST_F(video_writer_test, open_three_different_paths)
{
    const auto video_path1 = test_data_directory + "out1.mp4";
    ASSERT_TRUE(v->open(video_path1, width, height, fps));

    const auto video_path2 = test_data_directory + "out2.mp4";
    ASSERT_TRUE(v->open(video_path2, width, height, fps));

    const auto video_path3 = test_data_directory + "out3.mp4";
    ASSERT_TRUE(v->open(video_path3, width, height, fps));

    ASSERT_TRUE(v->is_opened());
}

TEST_P(video_writer_test, write_n_frames)
{
    const std::string format = GetParam();
    const auto video_path = test_data_directory + "out" + format;

    ASSERT_TRUE(v->open(video_path, width, height, fps));
    ASSERT_TRUE(v->is_opened());

    int num_written_frames = 0;
    const int num_frames_to_write = 300;
    while(num_written_frames < num_frames_to_write)
    {
        ASSERT_TRUE(v->write(frame_data.data()));
        num_written_frames++;
    }

    ASSERT_EQ(num_written_frames, num_frames_to_write);

    ASSERT_TRUE(v->save());
    ASSERT_FALSE(v->is_opened());

    // Check video info
}

TEST_P(video_writer_test, write_n_seconds)
{
    const std::string format = GetParam();
    const auto video_path = test_data_directory + "out" + format;

    const int duration_to_write_in_seconds = 10;
    ASSERT_TRUE(v->open(video_path, width, height, fps, duration_to_write_in_seconds));
    ASSERT_TRUE(v->is_opened());

    int num_written_frames = 0;
    while(true)
    {
        if(!v->write(frame_data.data()))
            break;

        num_written_frames++;
    }

    ASSERT_EQ(num_written_frames, duration_to_write_in_seconds * fps);

    ASSERT_TRUE(v->save());
    ASSERT_FALSE(v->is_opened());

    // Check video info
}

INSTANTIATE_TEST_SUITE_P(multi_format, video_writer_test, ::testing::Values(".mp4", ".mpeg", ".avi"));

/* 
TODO:
- N parallel writers
- always cleanup generated files (use TearDown?)

*/

}
