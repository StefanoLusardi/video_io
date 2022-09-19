#include <video_io/video_writer.hpp>
#include <video_io/raw_frame.hpp>

#include "logger.hpp"

extern "C"
{
#include <libavutil/log.h>
#include <libavutil/avassert.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavutil/dict.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
 
namespace vc
{
video_writer::video_writer() noexcept
: _is_opened { false }
{
    init(); 
    av_log_set_level(0);
}

video_writer::~video_writer() noexcept
{
    release();
}

void video_writer::init()
{
    _is_opened = false;
    
    _stream = nullptr;
    _stream_duration = -1;
    _next_pts = 0;

    _codec_ctx = nullptr;
    _frame = nullptr;
    _tmp_frame = nullptr;
    _packet = nullptr;
    _sws_ctx = nullptr;
    _format_ctx = nullptr;
}

bool video_writer::open(const std::string& video_path, int width, int height, const int fps, const int duration)
{
    std::lock_guard lock(_open_mutex);

    release();

    _stream_duration = duration;

    log_info("Opening video path:", video_path, "width:", width, "height:", height, "fps:", fps);

    if (auto r = avformat_alloc_output_context2(&_format_ctx, nullptr, nullptr, video_path.c_str()); r < 0) 
    {
        log_error("Could not deduce output format from file extension: using MPEG", vc::logger::get().err2str(r));
        
        if (auto r = avformat_alloc_output_context2(&_format_ctx, nullptr, "mpeg", video_path.c_str()); r < 0)
        {
            log_error("avformat_alloc_output_context2", vc::logger::get().err2str(r));
            return false;
        }
    }

    const AVCodec* codec = avcodec_find_encoder(_format_ctx->oformat->video_codec);
    if (!codec)
    {
        log_error("Could not find encoder for:", avcodec_get_name(_output_format->video_codec));
        return false;
    }

    if (_stream = avformat_new_stream(_format_ctx, nullptr); !_stream)
    {
        log_error("avformat_new_stream", vc::logger::get().err2str(r));
        return false;
    }
    _stream->id = _format_ctx->nb_streams-1;
    _stream->time_base = AVRational{ 1, fps }; // For fixed-fps content timebase should be 1/framerate
    _stream->r_frame_rate = AVRational{ fps, 1 };
    _stream->avg_frame_rate = AVRational{ fps, 1 };

    if (_codec_ctx = avcodec_alloc_context3(codec); !_codec_ctx)
    {
        log_error("avcodec_alloc_context3");
        return false;
    }
    _codec_ctx->codec_id = codec->id;
    _codec_ctx->bit_rate = 400000;
    _codec_ctx->width = width - (width % 2); // Keep sizes a multiple of 2
    _codec_ctx->height = height - (height % 2);
    _codec_ctx->time_base = _stream->time_base;
    _codec_ctx->gop_size = 12; // emit one intra frame every 12 frames at most
    _codec_ctx->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;

    if (_codec_ctx->codec_id == AV_CODEC_ID_MPEG2VIDEO)
    {
        _codec_ctx->max_b_frames = 2;
    }

    if (_codec_ctx->codec_id == AV_CODEC_ID_MPEG1VIDEO)
    {
        _codec_ctx->mb_decision = 2;
    }

    if (_format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        _codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (auto r = avcodec_open2(_codec_ctx, codec, nullptr); r < 0)
    {
        log_error("avcodec_open2", vc::logger::get().err2str(r));
        return false;
    }

    if (_packet = av_packet_alloc(); !_packet)
    {
        log_error("av_packet_alloc", vc::logger::get().err2str(r));
        return false;
    }

    if (_frame = alloc_frame(static_cast<int>(_codec_ctx->pix_fmt), _codec_ctx->width, _codec_ctx->height); !_frame)
    {
        log_error("alloc_frame");
        return false;
    }

    if (_codec_ctx->pix_fmt != AV_PIX_FMT_YUV420P)
    {
        if (_tmp_frame = alloc_frame(static_cast<int>(AVPixelFormat::AV_PIX_FMT_YUV420P), _codec_ctx->width, _codec_ctx->height); !_frame)
        {
            log_error("alloc_frame");
            return false;
        }
    }

    if (auto r = avcodec_parameters_from_context(_stream->codecpar, _codec_ctx); r < 0)
    {
        log_error("avcodec_parameters_from_context", vc::logger::get().err2str(r));
        return false;
    }

    if (!(_format_ctx->oformat->flags & AVFMT_NOFILE)) 
    {
        if (auto r = avio_open(&_format_ctx->pb, video_path.c_str(), AVIO_FLAG_WRITE); r < 0) 
        {
            log_error("avio_open", vc::logger::get().err2str(r));
            return false;
        }
    }
 
    if (auto r = avformat_write_header(_format_ctx, nullptr); r < 0) 
    {
        log_error("avformat_write_header", vc::logger::get().err2str(r));
        return false;
    }

    _is_opened = true;
    return true;
}

bool video_writer::is_opened() const
{
    return _is_opened;
}

AVFrame* video_writer::alloc_frame(int pix_fmt, int width, int height)
{
    AVFrame* frame;

    if (frame = av_frame_alloc(); !frame)
    {
        log_error("av_frame_alloc");
        return nullptr;
    }

    frame->format = pix_fmt;
    frame->width  = width;
    frame->height = height;
    if (auto r = av_frame_get_buffer(frame, 0); r < 0)
    {
        log_error("av_frame_get_buffer", vc::logger::get().err2str(r));
        return nullptr;
    }
 
    return frame;
}

bool video_writer::encode(AVFrame* frame)
{
    int ret = 0;
 
    if (auto r = avcodec_send_frame(_codec_ctx, frame); r < 0) 
    {
        log_error("avcodec_send_frame", vc::logger::get().err2str(r));
        return false;
    }
 
    while (ret >= 0) 
    {
        ret = avcodec_receive_packet(_codec_ctx, _packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;

        else if (ret < 0) 
        {
            exit(1);
        }
 
        av_packet_rescale_ts(_packet, _codec_ctx->time_base, _stream->time_base);
        _packet->stream_index = _stream->index;
 
        ret = av_interleaved_write_frame(_format_ctx, _packet);
        if (ret < 0) 
        {
            exit(1);
        }
    }
 
    return ret != AVERROR_EOF;


    
    // while (true) 
    // {        
    //     if (auto r = avcodec_receive_packet(_codec_ctx, _packet); r < 0)
    //     {
    //         if(r == AVERROR(EAGAIN)) // || r == AVERROR_EOF)
    //             return true;

    //         return false;
    //     }
 
    //     av_packet_rescale_ts(_packet, _codec_ctx->time_base, _stream->time_base);
    //     _packet->stream_index = _stream->index;
 
    //     if (auto r = av_interleaved_write_frame(_format_ctx, _packet); r < 0)
    //     {
    //         log_info("av_interleaved_write_frame", vc::logger::get().err2str(r));
    //         return false;
    //     }
    //     // _packet is now blank since av_interleaved_write_frame() takes ownership of its contents and resets _packet
    //     // unreferencing is not necessary i.e. no need to call av_packet_unref(_packet)
    // }

    // return true;
}

// AVFrame* video_writer::convert(const uint8_t* data)
bool video_writer::convert(const uint8_t* data)
{
    if (_stream_duration > 0 && av_compare_ts(_next_pts, _codec_ctx->time_base, _stream_duration, AVRational{ 1, 1 }) >= 0)
    {
        log_info("End of stream");
        // return nullptr;
        encode(nullptr);
        return false;
    }

    // when we pass a frame to the encoder, it may keep a reference to it internally; make sure we do not overwrite it here
    if (auto r = av_frame_make_writable(_frame); r < 0)
    {
        log_error("av_frame_make_writable", vc::logger::get().err2str(r));
        // return nullptr;
        return false;
    }

    /* Y */
    for (int y = 0; y < _codec_ctx->height; y++)
    {
        for (int x = 0; x < _codec_ctx->width; x++)
        {
            _frame->data[0][y * _frame->linesize[0] + x] = static_cast<uint8_t>((float)_next_pts/300*255);
        }
    }
 
    /* Cb and Cr */
    for (int y = 0; y < _codec_ctx->height / 2; y++)
    {
        for (int x = 0; x < _codec_ctx->width / 2; x++)
        {
            _frame->data[1][y * _frame->linesize[1] + x] = 127;
            _frame->data[2][y * _frame->linesize[2] + x] = 0;
        }
    }

    // if (auto r = av_image_fill_arrays(_frame->data, _frame->linesize, data, _codec_ctx->pix_fmt, _codec_ctx->width, _codec_ctx->height, 1); r < 0)
    // {
    //     log_error("av_image_fill_arrays", vc::logger::get().err2str(r));
    //     return nullptr;  
    //     // return false;  
    // }
    
    _frame->pts = _next_pts++; // Timestamp increment must be 1 for fixed-fps content
    
    // return _frame;
    return true;
}

bool video_writer::write(const uint8_t* data)
{
    if(!convert(data))
        return false;

    if(!encode(_frame))
        return false;

    return true;
}

bool video_writer::write(raw_frame* frame)
{
    // if(!convert(frame))
    //     return false;

    // if(!encode())
    //     return false;

    return true;
}

bool video_writer::flush()
{
    return true;
}

bool video_writer::save()
{
    if(!_is_opened)
        return false;

    encode(nullptr);

    if(auto r = av_write_trailer(_format_ctx); r < 0) 
    {
        log_error("avformat_write_header", vc::logger::get().err2str(r));
        return false;
    }

    if (!(_format_ctx->oformat->flags & AVFMT_NOFILE))
    {
        if (auto r = avio_closep(&_format_ctx->pb); r < 0) 
        {
            log_error("avformat_write_header", vc::logger::get().err2str(r));
            return false;
        }
    }

    release();

    return true;
}

void video_writer::release()
{
    if(!_is_opened)
        return;
    
    if(_codec_ctx)
        avcodec_free_context(&_codec_ctx);

    if(_frame)
        av_frame_free(&_frame);
    
    if(_tmp_frame)
        av_frame_free(&_tmp_frame);
    
    if(_packet)
        av_packet_free(&_packet);

    if(_sws_ctx)
        sws_freeContext(_sws_ctx);

    if(_format_ctx)
        avformat_free_context(_format_ctx);

    init();
}


/*
bool video_writer::encode()
{
    int ret;
 
    // send the frame to the encoder
    ret = avcodec_send_frame(_codec_ctx, _frame);
    if (ret < 0) 
    {
        // fprintf(stderr, "Error sending a frame to the encoder: %s\n", av_err2str(ret));
        return false;
    }

    while (ret >= 0) 
    {
        ret = avcodec_receive_packet(_codec_ctx, _packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;

        else if (ret < 0) 
        {
            // fprintf(stderr, "Error encoding a frame: %s\n", av_err2str(ret));
            return false;
        }

        // rescale output packet timestamp values from codec to stream timebase
        av_packet_rescale_ts(_packet, _codec_ctx->time_base, _stream->time_base);
        _packet->stream_index = _stream->index;
 
        // Write the compressed frame to the media file.
        ret = av_interleaved_write_frame(_format_ctx, _packet);

        // _packet is now blank (av_interleaved_write_frame() takes ownership of
        // its contents and resets _packet), so that no unreferencing is necessary.
        // This would be different if one used av_write_frame().
        if (ret < 0) 
        {
            // fprintf(stderr, "Error while writing output packet: %s\n", av_err2str(ret));
            return false;
        }
    }
 
    return ret != AVERROR_EOF;
}
*/
 
}

/*
static AVFrame *get_video_frame(OutputStream *ost)
{
    AVCodecContext *c = ost->enc;
 
    // check if we want to generate more frames
    if (av_compare_ts(ost->_next_pts, c->time_base, STREAM_DURATION, (AVRational){ 1, 1 }) > 0)
        return NULL;
 
    // when we pass a frame to the encoder, it may keep a reference to it internally; make sure we do not overwrite it here 
    if (av_frame_make_writable(ost->frame) < 0)
        exit(1);
 
    if (c->pix_fmt != AV_PIX_FMT_YUV420P)
    {
        // as we only generate a YUV420P picture, we must convert it to the codec pixel format if needed 
        if (!ost->sws_ctx) 
        {
            ost->sws_ctx = sws_getContext(
                c->width, c->height, AV_PIX_FMT_YUV420P,
                c->width, c->height, c->pix_fmt, SWS_BICUBIC, 
                NULL, NULL, NULL);

            if (!ost->sws_ctx) 
            {
                fprintf(stderr, "Could not initialize the conversion context\n");
                exit(1);
            }
        }

        fill_yuv_image(ost->tmp_frame, ost->_next_pts, c->width, c->height);

        sws_scale(ost->sws_ctx, (const uint8_t * const *) ost->tmp_frame->data,
            ost->tmp_frame->linesize, 0, c->height, ost->frame->data, ost->frame->linesize);
    }
    else 
    {
        fill_yuv_image(ost->frame, ost->_next_pts, c->width, c->height);
    }
 
    ost->frame->pts = ost->_next_pts++;
 
    return ost->frame;
}
*/