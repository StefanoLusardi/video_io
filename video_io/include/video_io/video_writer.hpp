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
struct AVOutputFormat;

namespace vc
{
struct raw_frame;

class API_VIDEO_IO video_writer
{
public:
    explicit video_writer() noexcept;
    ~video_writer() noexcept;
    
    // using log_callback_t = std::function<void(const std::string&)>;
    // void set_log_callback(const log_callback_t& cb, const log_level& level = log_level::all);    

    bool open(const std::string& video_path);
    bool is_opened() const;
    bool write(const uint8_t* data);
    // bool write(uint8_t** data);
    bool write(raw_frame* frame);
    void release();

    void save();
    
    // auto get_frame_count() const -> std::optional<int>;
    // auto get_duration() const -> std::optional<std::chrono::steady_clock::duration>;
    // auto get_frame_size() const -> std::optional<std::tuple<int, int>>;
    // auto get_frame_size_in_bytes() const -> std::optional<int>;
    // auto get_fps() const -> std::optional<double>;

protected:
    void init();
    bool add_stream();
    bool open_video();
    AVFrame* alloc_picture(int pix_fmt, int width, int height);
    bool write_frame();
    void close_stream();

    // bool encode();
    // bool convert();

private:
    bool _is_opened;
    std::mutex _open_mutex;

    int64_t next_pts;
    AVStream* _stream;
    
    AVFrame* _frame;
    AVFrame* _tmp_frame;

    SwsContext* _sws_ctx;

    const AVCodec* codec;
    const AVOutputFormat *_output_format;
    AVFormatContext* _format_ctx;
    AVCodecContext* _codec_ctx;
    AVPacket* _packet;
};

}
