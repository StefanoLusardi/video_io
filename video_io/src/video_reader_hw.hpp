#pragma once

#include "logger.hpp"
#include <video_io/video_reader.hpp>

extern "C"
{
#include <libavutil/hwcontext.h>
}

namespace vc
{
struct video_reader::hw_acceleration
{
    explicit hw_acceleration();
    ~hw_acceleration();

    decode_support init();
    AVBufferRef* get_frames_ctx(int w, int h);
    void release();
    void reset();

    AVBufferRef* hw_device_ctx;
    AVBufferRef* hw_frames_ctx;
    int hw_pixel_format;
};

}
