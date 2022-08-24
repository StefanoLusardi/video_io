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
struct AVDictionary;
struct SwsContext;
struct AVBufferRef;
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
    bool write(uint8_t** data);
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
    bool write_frame();
    // bool decode();
    // bool retrieve();
    // bool is_error(const char* func_name, const int error) const;

private:
    bool _is_opened;
    std::mutex _open_mutex;

    AVStream* _stream;
 
    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;
 
    AVFrame *frame;
 
    float t, tincr, tincr2;
 
    SwsContext* _sws_ctx;

    const AVOutputFormat *_output_format;
    AVFormatContext* _format_ctx;
    AVCodecContext* _codec_ctx;
    AVPacket* _packet;
    
    // AVFrame* _src_frame;
    // AVFrame* _dst_frame;
    AVFrame* _tmp_frame;
    
    AVDictionary* _options;
    int _stream_index;
    double _timestamp_unit;
};

}
