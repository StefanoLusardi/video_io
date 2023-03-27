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
struct AVDictionary;
struct AVInputFormat;

namespace vio
{
struct simple_frame;
enum class decode_support { none, SW, HW };
struct screen_options{};

class API_VIDEO_IO video_reader
{
public:
    explicit video_reader() noexcept;
    ~video_reader() noexcept;
    
    using log_callback_t = std::function<void(const std::string&)>;
    // void set_log_callback(const log_callback_t& cb, const log_level& level = log_level::all);

    bool open(const char* video_path, decode_support decode_preference = decode_support::none);
    bool open(const char* screen_name, screen_options screen_opt);
    bool is_opened() const;
    bool read(uint8_t** data, double* pts = nullptr);
    bool release();
    
    auto get_frame_count() const -> std::optional<int>;
    auto get_duration() const -> std::optional<std::chrono::steady_clock::duration>;
    auto get_frame_size() const -> std::optional<std::tuple<int, int>>;
    auto get_frame_size_in_bytes() const -> std::optional<int>;
    auto get_fps() const -> std::optional<double>;

protected:
    void init();
    bool open_input(const char* input, const AVInputFormat* input_format);
    bool decode(AVPacket *packet);
    bool convert(uint8_t** data, double* pts);
    bool copy_hw_frame();

private:
    bool _is_opened;
    std::mutex _open_mutex;
    decode_support _decode_support;

    AVFormatContext* _format_ctx;
    AVCodecContext* _codec_ctx; 
    AVPacket* _packet;
    SwsContext* _sws_ctx;
    
    AVFrame* _src_frame;
    AVFrame* _dst_frame;
    AVFrame* _tmp_frame;
    
    AVDictionary* _options;
    int _stream_index;

    class hw_acceleration;
    std::unique_ptr<hw_acceleration> _hw;
};

}
