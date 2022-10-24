#pragma once 

#include <gtest/gtest.h>
#include <video_io/video_writer.hpp>

#include <array>
#include <algorithm>
#include <filesystem>

namespace vc::test
{

class video_writer_test : public ::testing::TestWithParam<std::string>
{
public:
    static void SetUpTestSuite() 
    {
    }

    static void TearDownTestSuite() 
    {
    }

protected:
    explicit video_writer_test()
    : v{ std::make_unique<vc::video_writer>() }
    , test_name { testing::UnitTest::GetInstance()->current_test_info()->name() }
    , default_output_directory{ std::filesystem::current_path() / "temp" }
    , default_video_extension { ".mp4" }
    , default_video_path { (default_output_directory / test_name).replace_extension(default_video_extension) }
    {         
        frame_data.fill(static_cast<uint8_t>(0));
    }

    virtual ~video_writer_test() { v->release(); }

    virtual void SetUp() override { }
    virtual void TearDown() override { }

    std::unique_ptr<vc::video_writer> v;
    const std::string test_name;
    const std::filesystem::path default_output_directory;
    const std::string default_video_extension;
    const std::filesystem::path default_video_path;

    static const int fps = 30;
    static const int width = 640;
	static const int height = 480;
	static const int duration = 2;

    static const int frame_size = width * height * 3;
    std::array<uint8_t, frame_size> frame_data = { };

    // Standard Definition (SD): 640 x 480
    // High Definition (HD): 1280 x 720
    // Full HD (FHD): 1920 x 1080
    // 4K/Ultra HD (UHD): 3840 x 2160

private:
    template<typename... Args>
    void log(Args&&... args) const
    {
        ((std::cout << std::forward<Args>(args) << ' ') , ...) << std::endl;
    }
};

}
