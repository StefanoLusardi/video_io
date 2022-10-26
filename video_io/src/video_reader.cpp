#include <video_io/video_reader.hpp>
#include <video_io/raw_frame.hpp>

#include "logger.hpp"
#include "video_reader_hw.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <libavutil/buffer.h>
#include <libavutil/hwcontext.h>
}

namespace vc
{
video_reader::video_reader() noexcept
: _is_opened{ false }
{
    init(); 
    av_log_set_level(0);
}

video_reader::~video_reader() noexcept
{
    release();
}


void video_reader::init()
{
    log_info("Reset video capture");

    _is_opened = false;
    _decode_support = decode_support::none;
    
    _format_ctx = nullptr;
    _codec_ctx = nullptr; 
    _packet = nullptr;

    _src_frame = nullptr;
    _dst_frame = nullptr;
    _tmp_frame = nullptr;

    _sws_ctx = nullptr;
    _options = nullptr;
    _stream_index = -1;
}

// void video_reader::set_log_callback(const log_callback_t& cb, const log_level& level) { vc::logger::get().set_log_callback(cb, level); }

bool video_reader::open(const std::string& video_path, decode_support decode_preference)
{
    std::lock_guard lock(_open_mutex);
    release();

    log_info("Opening video path:", video_path);
    log_info("HW acceleration", (decode_preference == decode_support::HW ? "required" : "not required"));

    if(decode_preference == decode_support::HW)
    {
        _hw = std::make_unique<hw_acceleration>();
        _decode_support = _hw->init();
    }
    else
    {
        _decode_support = decode_support::SW;
    }

    if (_format_ctx = avformat_alloc_context(); !_format_ctx)
    {
        log_error("avformat_alloc_context");
        return false;
    }

    if (auto r = av_dict_set(&_options, "rtsp_transport", "tcp", 0); r < 0)
    {
        log_error("av_dict_set", vc::logger::get().err2str(r));
        return false;
    }

    if (auto r = avformat_open_input(&_format_ctx, video_path.c_str(), nullptr, &_options); r < 0)
    {
        log_error("avformat_open_input", vc::logger::get().err2str(r));
        return false;
    }

    if (auto r = avformat_find_stream_info(_format_ctx, nullptr); r < 0)
    {
        log_error("avformat_find_stream_info");
        return false;
    }

    const AVCodec* codec = nullptr;
    if (_stream_index = av_find_best_stream(_format_ctx, AVMediaType::AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0); _stream_index < 0)
    {
        log_error("av_find_best_stream", vc::logger::get().err2str(_stream_index));
        return false;
    }

    if (_codec_ctx = avcodec_alloc_context3(codec); !_codec_ctx)
    {
        log_error("avcodec_alloc_context3");
        return false;
    }

    if (auto r = avcodec_parameters_to_context(_codec_ctx, _format_ctx->streams[_stream_index]->codecpar); r < 0)
    {
        log_error("avcodec_parameters_to_context", vc::logger::get().err2str(r));
        return false;
    }

    if(_decode_support == decode_support::HW)
    {
        // _codec_ctx->sw_pix_fmt = AV_PIX_FMT_NV12;
        _codec_ctx->hw_device_ctx = av_buffer_ref(_hw->hw_device_ctx);
        // _codec_ctx->hw_frames_ctx = _hw->get_frames_ctx(_codec_ctx->width, _codec_ctx->height);
    }

    if (auto r = avcodec_open2(_codec_ctx, codec, nullptr); r < 0)
    {
        log_error("avcodec_open2", vc::logger::get().err2str(r));
        return false;
    }

    if (_packet = av_packet_alloc(); !_packet)
    {
        log_error("av_packet_alloc");
        return false;
    }
    
    if (_src_frame = av_frame_alloc(); !_src_frame)
    {
        log_error("av_frame_alloc");
        return false;
    }

    if (_dst_frame = av_frame_alloc(); !_dst_frame)
    {
        log_error("av_frame_alloc");
        return false;
    }

    if(_decode_support == decode_support::HW)
    {
        // HW: Allocate one extra frame for HW decoding.
        if (_tmp_frame = av_frame_alloc(); !_tmp_frame)
        {
            log_error("av_frame_alloc");
            return false;
        }
    }
    else
    {
        // SW: No need to allocate any temporary frame, just make it point to _src_frame.
        _tmp_frame = _src_frame;
    }

    _dst_frame->format = AVPixelFormat::AV_PIX_FMT_BGR24;
	_dst_frame->width  = _codec_ctx->width;
	_dst_frame->height = _codec_ctx->height;
    if (auto r = av_frame_get_buffer(_dst_frame, 0); r < 0)
    {
        log_error("av_frame_get_buffer", vc::logger::get().err2str(r));
        return false;
    }

    _is_opened = true;
    log_info("Video Reader is opened correctly");
    return true;
}

bool video_reader::is_opened() const
{
    return _is_opened;
}

auto video_reader::get_frame_count() const -> std::optional<int>
{
    if(!_is_opened)
    {
        log_error("Frame count not available. Video path must be opened first.");
        return std::nullopt;
    }

    auto nb_frames = _format_ctx->streams[_stream_index]->nb_frames;
    if (!nb_frames)
    {
        double duration_sec = static_cast<double>(_format_ctx->duration) / static_cast<double>(AV_TIME_BASE);
        auto fps = get_fps();
        nb_frames = std::floor(duration_sec * fps.value() + 0.5);
    }
    if (nb_frames)
        return std::make_optional(static_cast<int>(nb_frames));
    
    return std::nullopt;
}

auto video_reader::get_duration() const -> std::optional<std::chrono::steady_clock::duration>
{
    if(!_is_opened)
    {
        log_error("Duration not available. Video path must be opened first.");
        return std::nullopt;
    }

    auto duration = std::chrono::duration<int64_t, std::ratio<1, AV_TIME_BASE>>(_format_ctx->duration);
    return std::make_optional(duration);
}

auto video_reader::get_frame_size() const -> std::optional<std::tuple<int, int>>
{
    if(!_is_opened)
    {
        log_error("Frame size not available. Video path must be opened first.");
        return std::nullopt;
    }
    
    auto size = std::make_tuple(_codec_ctx->width, _codec_ctx->height);
    return std::make_optional(size);
}

auto video_reader::get_frame_size_in_bytes() const -> std::optional<int>
{
    if(!_is_opened)
    {
        log_error("Frame size in bytes not available. Video path must be opened first.");
        return std::nullopt;
    }

    auto bytes = _codec_ctx->width * _codec_ctx->height * 3;
    return std::make_optional(bytes);
}

auto video_reader::get_fps() const -> std::optional<double>
{
    if(!_is_opened)
    {
        log_error("FPS not available. Video path must be opened first.");
        return std::nullopt;
    }
    
    auto frame_rate = _format_ctx->streams[_stream_index]->avg_frame_rate;
    if(frame_rate.num <= 0 || frame_rate.den <= 0 )
    {
        log_info("Unable to convert FPS.");
        return std::nullopt;
    }

    auto fps = static_cast<double>(frame_rate.num) / static_cast<double>(frame_rate.den);
    return std::make_optional(fps);
}

bool video_reader::decode(AVPacket *packet)
{
    if(auto r = av_read_frame(_format_ctx, packet); r < 0)
    {
        log_error("av_read_frame", vc::logger::get().err2str(r));
        return false;
    }

    while(true)
    {
        if (packet->stream_index != _stream_index)
        {
            av_packet_unref(packet);
            continue;
        }

        if (auto r = avcodec_send_packet(_codec_ctx, packet); r < 0)
        {
            log_error("avcodec_send_packet", r);
            av_packet_unref(packet);
            return false;
        }

        if (auto r = avcodec_receive_frame(_codec_ctx, _src_frame); r < 0)
        {
            if (AVERROR(EAGAIN) == r)
                continue; 
            
            av_packet_unref(packet);
            log_info("avcodec_receive_frame", vc::logger::get().err2str(r));
            return false;
        }
        
        av_packet_unref(packet);
        return true;
    }
}

/*
        decode(nullptr);

bool video_reader::decode(AVPacket *packet)
{
    if(auto r = av_read_frame(_format_ctx, _packet); r < 0)
    {
        log_error("av_read_frame", vc::logger::get().err2str(r));
        return false;
    }

    while(true)
    {
        if (_packet->stream_index != _stream_index)
        {
            av_packet_unref(_packet);
            continue;
        }

        if (auto r = avcodec_send_packet(_codec_ctx, _packet); r < 0)
        {        
            log_error("avcodec_send_packet", r);
            return false;
        }

        if (auto r = avcodec_receive_frame(_codec_ctx, _src_frame); r < 0)
        {
            if(r == AVERROR_EOF || r == AVERROR(EAGAIN))
            {
                av_packet_unref(_packet);
                continue;
            }

            return false;
        }
    }

    av_packet_unref(_packet);
    return true;
}
*/

bool video_reader::copy_hw_frame()
{
    if (_src_frame->format == _hw->hw_pixel_format)
    {
        if (auto r = av_hwframe_transfer_data(_tmp_frame, _src_frame, 0); r < 0)
        {
            log_error("av_hwframe_transfer_data", vc::logger::get().err2str(r));
            return false;
        }

        if (auto r = av_frame_copy_props(_tmp_frame, _src_frame); r < 0)
        {
            log_error("av_frame_copy_props", vc::logger::get().err2str(r));
            return false;
        }
    }
    else
    {
        _tmp_frame = _src_frame;
    }

    return true;
}

bool video_reader::convert(uint8_t** data)
{   
    if(_decode_support == decode_support::HW)
    {
        if(!copy_hw_frame())
            return false;
    }

    if (!_sws_ctx)
    {
        _sws_ctx = sws_getCachedContext(_sws_ctx,
            _codec_ctx->width, _codec_ctx->height, (AVPixelFormat)_tmp_frame->format,
            _codec_ctx->width, _codec_ctx->height, AVPixelFormat::AV_PIX_FMT_BGR24,
            SWS_BICUBIC, nullptr, nullptr, nullptr);
        
        if (!_sws_ctx)
        {
            log_error("Unable to initialize SwsContext");
            return false;
        }
    }

    // _dst_frame->linesize[0] = _codec_ctx->width * 3;
    sws_scale(_sws_ctx, _tmp_frame->data, _tmp_frame->linesize,
        0, _codec_ctx->height, _dst_frame->data, _dst_frame->linesize);

    *data = _dst_frame->data[0];

    const auto time_base = _format_ctx->streams[_stream_index]->time_base;
    const auto pts = _tmp_frame->best_effort_timestamp * static_cast<double>(time_base.num) / static_cast<double>(time_base.den);

    return true;
}

bool video_reader::read(uint8_t** data)
{
    if(!_is_opened)
        return false;

    if(!decode(_packet))
        return false;

    if(!convert(data))
        return false;

    return true;
}

bool video_reader::read(raw_frame* frame)
{
    if(!_is_opened)
        return false;
    
    // if(!decode())
    //     return false;

    // if(!copy_hw_frame())
    //     return false;

    // _dst_frame->data[0] = frame->data.data();
    // if(!convert())
    //     return false;

    // const auto time_base = _format_ctx->streams[_stream_index]->time_base;
    // frame->pts = _tmp_frame->best_effort_timestamp * static_cast<double>(time_base.num) / static_cast<double>(time_base.den);
    
    return true;
}

bool video_reader::release()
{
    if(!_is_opened)
        return false;

    log_info("Release video reader");

    if(_sws_ctx)
        sws_freeContext(_sws_ctx);

    if(_codec_ctx)
        avcodec_free_context(&_codec_ctx);

    if(_format_ctx)
    {
        avformat_close_input(&_format_ctx);
        avformat_free_context(_format_ctx);
    }

    if (_options)
       av_dict_free(&_options);

    if(_packet)
        av_packet_free(&_packet);

    if(_src_frame)
        av_frame_free(&_src_frame);

    if(_dst_frame)
        av_frame_free(&_dst_frame);

    if(_tmp_frame && _decode_support == decode_support::HW)
        av_frame_free(&_tmp_frame);

    init();

    if(_decode_support == decode_support::HW)
        _hw->release();

    return true;
}

}
