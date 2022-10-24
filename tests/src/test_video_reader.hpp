#pragma once 

#include <gtest/gtest.h>
#include <video_io/video_reader.hpp>

#include <filesystem>

namespace vc::test
{

class video_reader_test : public ::testing::TestWithParam<std::string>
{
protected:
    explicit video_reader_test()
    : v{ std::make_unique<vc::video_reader>() }
    , test_name { testing::UnitTest::GetInstance()->current_test_info()->name() }
    , default_input_directory{ std::filesystem::current_path() / "../../../tests/data/new" }
    , default_video_extension { ".mp4" }
    , default_video_name { "testsrc2_3sec_30fps_640x480" }
    , default_video_path { (default_input_directory / default_video_name).replace_extension(default_video_extension) }
    { }

    virtual ~video_reader_test() { v->release(); }

    virtual void SetUp() override { }
    virtual void TearDown() override { }

    std::unique_ptr<vc::video_reader> v;
    const std::string test_name;
    const std::filesystem::path default_input_directory;
    const std::string default_video_extension;
    const std::string default_video_name;
    const std::filesystem::path default_video_path;

    static const int fps = 30;
    static const int width = 640;
	static const int height = 480;

    static const int frame_size = width * height * 3;
    std::array<uint8_t, frame_size> frame_data = { };

private:
    template<typename... Args>
    void log(Args&&... args) const
    {
        ((std::cout << std::forward<Args>(args) << ' ') , ...) << std::endl;
    }
};

}
