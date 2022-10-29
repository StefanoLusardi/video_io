#pragma once

#include "api.hpp"

#include <string>
#include <functional>
#include <memory>
#include <optional>
#include <chrono>
#include <mutex>

struct AVFormatContext;
struct AVCodecContext; 
struct AVCodec;
struct AVPacket;
struct AVFrame;
struct SwsContext;
struct AVStream;

namespace vio
{
struct simple_frame;

class API_VIDEO_IO video_writer
{
public:
    explicit video_writer() noexcept;
    ~video_writer() noexcept;
    
    // using log_callback_t = std::function<void(const std::string&)>;
    // void set_log_callback(const log_callback_t& cb, const log_level& level = log_level::all);    

    bool open(const std::string& video_path, int width, int height, const int fps);
    bool open(const std::string& video_path, int width, int height, const int fps, const int duration);
    bool is_opened() const;
    bool write(const uint8_t* data);
    bool write(simple_frame* frame);
    bool release();
    bool save();
    
    bool check(const std::string& video_path);

protected:
    void init();
    bool convert(const uint8_t* data);
    bool encode(AVFrame* frame);

    AVFrame* alloc_frame(int pix_fmt, int width, int height);

private:
    bool _is_opened;
    std::mutex _open_mutex;

    AVFormatContext* _format_ctx;
    AVCodecContext* _codec_ctx;
    AVPacket* _packet;
    SwsContext* _sws_ctx;

    AVFrame* _frame;
    AVFrame* _tmp_frame;

    AVStream* _stream;
    int64_t _stream_duration;
    int64_t _next_pts;
    
};

}
