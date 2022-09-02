#define __STDC_CONSTANT_MACROS

#include <video_io/video_writer.hpp>

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
 
#define STREAM_DURATION   10.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

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

}

bool video_writer::open(const std::string& video_path)
{
    std::lock_guard lock(_open_mutex);

    /* allocate the output media context */
    avformat_alloc_output_context2(&_format_ctx, NULL, NULL, video_path.c_str());
    if (!_format_ctx) 
    {
        // printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&_format_ctx, NULL, "mpeg", video_path.c_str());
    }

    if (!_format_ctx)
        return 1;
 
    _output_format = _format_ctx->oformat;

    if(!add_stream())
        return false;

    if (!open_video())
        return false;

    av_dump_format(_format_ctx, 0, video_path.c_str(), 1);

    /* open the output file, if needed */
    if (!(_output_format->flags & AVFMT_NOFILE)) 
    {
        int ret = avio_open(&_format_ctx->pb, video_path.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) 
        {
            // fprintf(stderr, "Could not open '%s': %s\n", filename, av_err2str(ret));
            return false;
        }
    }
 
    /* Write the stream header, if any. */
    int ret = avformat_write_header(_format_ctx, nullptr);
    if (ret < 0) 
    {
        // fprintf(stderr, "Error occurred when opening output file: %s\n", av_err2str(ret));
        return false;
    }

    _is_opened = true;
    return true;
}

bool video_writer::add_stream()
{
    codec = avcodec_find_encoder(_output_format->video_codec);
    if (!codec)
    {
        // fprintf(stderr, "Could not find encoder for '%s'\n", avcodec_get_name(_output_format->video_codec));
        return false;
    }

    // printf("encoder '%s'\n", avcodec_get_name(_output_format->video_codec));
 
    _packet = av_packet_alloc();
    if (!_packet)
    {
        // fprintf(stderr, "Could not allocate AVPacket\n");
        return false;
    }
 
    _stream = avformat_new_stream(_format_ctx, NULL);
    if (!_stream)
    {
        // fprintf(stderr, "Could not allocate stream\n");
        return false;
    }

    _stream->id = _format_ctx->nb_streams-1;

    _codec_ctx = avcodec_alloc_context3(codec);
    if (!_codec_ctx) 
    {
        fprintf(stderr, "Could not alloc an encoding context\n");
        return false;
    }

    _codec_ctx->codec_id = _output_format->video_codec;
    _codec_ctx->bit_rate = 400000;

    /* Resolution must be a multiple of two. */
    _codec_ctx->width    = 352;
    _codec_ctx->height   = 288;

    /* timebase: This is the fundamental unit of time (in seconds) in terms
        * of which frame timestamps are represented. For fixed-fps content,
        * timebase should be 1/framerate and timestamp increments should be
        * identical to 1. */
    _stream->time_base = (AVRational){ 1, STREAM_FRAME_RATE };
    
    _codec_ctx->time_base       = _stream->time_base;
    _codec_ctx->gop_size      = 12; /* emit one intra frame every twelve frames at most */
    _codec_ctx->pix_fmt       = STREAM_PIX_FMT;

    if (_codec_ctx->codec_id == AV_CODEC_ID_MPEG2VIDEO)
    {
        /* just for testing, we also add B-frames */
        _codec_ctx->max_b_frames = 2;
    }

    if (_codec_ctx->codec_id == AV_CODEC_ID_MPEG1VIDEO)
    {
        /* Needed to avoid using macroblocks in which some coeffs overflow.
            * This does not happen with normal video, it just happens here as
            * the motion of the chroma plane does not match the luma plane. */
        _codec_ctx->mb_decision = 2;
    }

    /* Some formats want stream headers to be separate. */
    if (_format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        _codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    return true;
}

bool video_writer::open_video()
{
    int ret;
 
    /* open the codec */
    ret = avcodec_open2(_codec_ctx, codec, nullptr);
    if (ret < 0)
    {
        // fprintf(stderr, "Could not open video codec: %s\n", av_err2str(ret));
        return false;
    }
 
    /* allocate and init a re-usable frame */
    _frame = alloc_picture(static_cast<int>(_codec_ctx->pix_fmt), _codec_ctx->width, _codec_ctx->height);
    if (!_frame)
    {
        // fprintf(stderr, "Could not allocate video frame\n");
        return false;
    }
 
    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    _tmp_frame = NULL;
    if (_codec_ctx->pix_fmt != AV_PIX_FMT_YUV420P)
    {
        _tmp_frame = alloc_picture(static_cast<int>(AV_PIX_FMT_YUV420P), _codec_ctx->width, _codec_ctx->height);
        if (!_tmp_frame)
        {
            // fprintf(stderr, "Could not allocate temporary picture\n");
            return false;
        }
    }
 
    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(_stream->codecpar, _codec_ctx);
    if (ret < 0)
    {
        // fprintf(stderr, "Could not copy the stream parameters\n");
        return false;
    }

    return true;
}

AVFrame* video_writer::alloc_picture(int pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;
 
    picture = av_frame_alloc();
    if (!picture)
        return NULL;
 
    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;
 
    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 0);
    if (ret < 0)
    {
        // fprintf(stderr, "Could not allocate frame data.\n");
        return nullptr;
    }
 
    return picture;
}

bool video_writer::is_opened() const
{
    return _is_opened;
}

bool video_writer::write(const uint8_t* data)
{
    av_image_fill_arrays(_frame->data, _frame->linesize, data, _codec_ctx->pix_fmt, _codec_ctx->width, _codec_ctx->height, 1);
    
    _frame->pts = next_pts++;
    return write_frame();
}

bool video_writer::write(raw_frame* frame)
{
    // convert();
    // encode();

    return false;
}

void video_writer::save()
{
    av_write_trailer(_format_ctx);
 
    // TODO: check if this is required at this line or it can be merged into release() called later in this function.
    close_stream();
 
    if (!(_output_format->flags & AVFMT_NOFILE))
        avio_closep(&_format_ctx->pb);

    release();
}

void video_writer::close_stream()
{
    avcodec_free_context(&_codec_ctx);
    av_frame_free(&_frame);
    av_frame_free(&_tmp_frame);
    av_packet_free(&_packet);
    sws_freeContext(_sws_ctx);
}

void video_writer::release()
{
    // avcodec_free_context(&_codec_ctx);
    // av_frame_free(&_frame);
    // av_frame_free(&_tmp_frame);
    // av_packet_free(&_packet);
    // sws_freeContext(_sws_ctx);

    avformat_free_context(_format_ctx);

    _is_opened = false;
}

bool video_writer::write_frame()
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
 
        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(_packet, _codec_ctx->time_base, _stream->time_base);
        _packet->stream_index = _stream->index;
 
        /* Write the compressed frame to the media file. */
        ret = av_interleaved_write_frame(_format_ctx, _packet);

        /* _packet is now blank (av_interleaved_write_frame() takes ownership of
         * its contents and resets _packet), so that no unreferencing is necessary.
         * This would be different if one used av_write_frame(). */
        if (ret < 0) 
        {
            // fprintf(stderr, "Error while writing output packet: %s\n", av_err2str(ret));
            return false;
        }
    }
 
    return ret != AVERROR_EOF;
}
 
}

/*
static AVFrame *get_video_frame(OutputStream *ost)
{
    AVCodecContext *c = ost->enc;
 
    // check if we want to generate more frames
    if (av_compare_ts(ost->next_pts, c->time_base, STREAM_DURATION, (AVRational){ 1, 1 }) > 0)
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

        fill_yuv_image(ost->tmp_frame, ost->next_pts, c->width, c->height);

        sws_scale(ost->sws_ctx, (const uint8_t * const *) ost->tmp_frame->data,
            ost->tmp_frame->linesize, 0, c->height, ost->frame->data, ost->frame->linesize);
    }
    else 
    {
        fill_yuv_image(ost->frame, ost->next_pts, c->width, c->height);
    }
 
    ost->frame->pts = ost->next_pts++;
 
    return ost->frame;
}
*/