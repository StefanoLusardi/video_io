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
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
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
    AVFormatContext *_format_ctx;

    /* allocate the output media context */
    avformat_alloc_output_context2(&_format_ctx, NULL, NULL, video_path.c_str());
    if (!_format_ctx) 
    {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&_format_ctx, NULL, "mpeg", video_path.c_str());
    }

    if (!_format_ctx)
        return 1;
 
    _output_format = _format_ctx->oformat;

    const AVCodec *video_codec;
    add_stream(&_stream, _format_ctx, &video_codec, _output_format->video_codec);

    open_video(_format_ctx, video_codec, &_stream, opt);
 
    av_dump_format(_format_ctx, 0, video_path.c_str(), 1);

    AVDictionary *opt = NULL;
 
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
    int ret = avformat_write_header(_format_ctx, &opt);
    if (ret < 0) 
    {
        // fprintf(stderr, "Error occurred when opening output file: %s\n", av_err2str(ret));
        return false;
    }

    _is_opened = true;
    return true;
}

bool video_writer::is_opened() const
{
    return _is_opened;
}

bool video_writer::write(uint8_t** data)
{
    get_video_frame();
    return write_frame();
}

bool video_writer::write(raw_frame* frame)
{
    return false;
}

void video_writer::save()
{
    av_write_trailer(_format_ctx);
 
    /* Close each codec. */
    close_stream(_format_ctx, &video_st);
 
    if (!(_output_format->flags & AVFMT_NOFILE))
        /* Close the output file. */
        avio_closep(&_format_ctx->pb);

    release();
}

void video_writer::release()
{
    avcodec_free_context(&_codec_ctx);
    av_frame_free(&frame);
    av_frame_free(&_tmp_frame);
    av_packet_free(&_packet);
    sws_freeContext(_sws_ctx);

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
        exit(1);
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
        log_packet(_format_ctx, _packet);
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
* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

static void log_packet(const AVFormatContext *_format_ctx, const AVPacket *pkt)
{
    AVRational *time_base = &_format_ctx->streams[pkt->stream_index]->time_base;
 
    // printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
    //        av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
    //        av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
    //        av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
    //        pkt->stream_index);
}

static void add_stream(OutputStream *ost, AVFormatContext *_format_ctx, const AVCodec **codec, enum AVCodecID codec_id)
{
    AVCodecContext *c;
    int i;
 
    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        fprintf(stderr, "Could not find encoder for '%s'\n",
                avcodec_get_name(codec_id));
        exit(1);
    }

    printf("encoder '%s'\n", avcodec_get_name(codec_id));
 
    ost->_packet = av_packet_alloc();
    if (!ost->_packet) {
        fprintf(stderr, "Could not allocate AVPacket\n");
        exit(1);
    }
 
    ost->_stream = avformat_new_stream(_format_ctx, NULL);
    if (!ost->_stream) {
        fprintf(stderr, "Could not allocate stream\n");
        exit(1);
    }
    ost->_stream->id = _format_ctx->nb_streams-1;
    c = avcodec_alloc_context3(*codec);
    if (!c) {
        fprintf(stderr, "Could not alloc an encoding context\n");
        exit(1);
    }
    ost->_codec_ctx = c;
 
    switch ((*codec)->type) {
    
    case AVMEDIA_TYPE_VIDEO:
        c->codec_id = codec_id;
 
        c->bit_rate = 400000;
        /* Resolution must be a multiple of two. */
        c->width    = 352;
        c->height   = 288;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
         * of which frame timestamps are represented. For fixed-fps content,
         * timebase should be 1/framerate and timestamp increments should be
         * identical to 1. */
        ost->_stream->time_base = (AVRational){ 1, STREAM_FRAME_RATE };
        c->time_base       = ost->_stream->time_base;
 
        c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
        c->pix_fmt       = STREAM_PIX_FMT;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            /* just for testing, we also add B-frames */
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
             * This does not happen with normal video, it just happens here as
             * the motion of the chroma plane does not match the luma plane. */
            c->mb_decision = 2;
        }
        break;
 
    default:
        break;
    }
 
    /* Some formats want stream headers to be separate. */
    if (_format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}
 
static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
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
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }
 
    return picture;
}
 
static void open_video(AVFormatContext *_format_ctx, const AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    int ret;
    AVCodecContext *c = ost->_codec_ctx;
    AVDictionary *opt = NULL;
 
    av_dict_copy(&opt, opt_arg, 0);
 
    /* open the codec */
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        // fprintf(stderr, "Could not open video codec: %s\n", av_err2str(ret));
        exit(1);
    }
 
    /* allocate and init a re-usable frame */
    ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
 
    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    ost->_tmp_frame = NULL;
    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        ost->_tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
        if (!ost->_tmp_frame) {
            fprintf(stderr, "Could not allocate temporary picture\n");
            exit(1);
        }
    }
 
    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->_stream->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }
}
 
static void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height)
{
    int x, y;
 
    double increment = (frame_index ) / (STREAM_DURATION * STREAM_FRAME_RATE);

    using DataT = decltype(pict->data[0]);

    /* Y */
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pict->data[0][y * pict->linesize[0] + x] = static_cast<uint8_t>(255 * increment);
        }
    }
 
    /* Cb and Cr */
    for (y = 0; y < height / 2; y++)
    {
        for (x = 0; x < width / 2; x++)
        {
            pict->data[1][y * pict->linesize[1] + x] = 127;
            pict->data[2][y * pict->linesize[2] + x] = 0;
        }
    }
}
 
static AVFrame *get_video_frame()
{
    /* check if we want to generate more frames */
    if (av_compare_ts(next_pts, _codec_ctx->time_base, STREAM_DURATION, (AVRational){ 1, 1 }) > 0)
        return NULL;
 
    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally; make sure we do not overwrite it here */
    if (av_frame_make_writable(_frame) < 0)
        exit(1);
 
    if (_codec_ctx->pix_fmt != AV_PIX_FMT_YUV420P)
    {
        /* as we only generate a YUV420P picture, we must convert it
         * to the codec pixel format if needed */
        if (!_sws_ctx) 
        {
            _sws_ctx = sws_getContext(
                _codec_ctx->width, _codec_ctx->height, AV_PIX_FMT_YUV420P,
                _codec_ctx->width, _codec_ctx->height, _codec_ctx->pix_fmt, SWS_BICUBIC, 
                NULL, NULL, NULL);

            if (!_sws_ctx) 
            {
                // fprintf(stderr, "Could not initialize the conversion context\n");
                exit(1);
            }
        }

        fill_yuv_image(_tmp_frame, next_pts, _codec_ctx->width, _codec_ctx->height);

        sws_scale(_sws_ctx, (const uint8_t * const *) _tmp_frame->data,
            _tmp_frame->linesize, 0, _codec_ctx->height, frame->data, frame->linesize);
    }
    else 
    {
        fill_yuv_image(frame, next_pts, _codec_ctx->width, _codec_ctx->height);
    }
 
    frame->pts = next_pts++;
 
    return frame;
}

