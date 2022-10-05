#pragma once 

#include <gtest/gtest.h>
#include <video_io/video_reader.hpp>


namespace vc::test
{

class video_reader_test : public ::testing::Test
{
protected:
    explicit video_reader_test()
    : v{ std::make_unique<vc::video_reader>() }
    , test_data_directory{"../data/"}
    { }

    virtual ~video_reader_test() { v->release(); }

    virtual void SetUp() override { }
    virtual void TearDown() override { }

    std::unique_ptr<vc::video_reader> v;
    const std::string test_data_directory;

private:
    template<typename... Args>
    void log(Args&&... args) const
    {
        ((std::cout << std::forward<Args>(args) << ' ') , ...) << std::endl;
    }
};

}
