/**
 * example: 	video_writer_simple_encode
 * author:		Stefano Lusardi
 * date:		Sep 2022
 * description:	The simplest example to show video_writer API usage. 
 * 				Single threaded: main thread is responsible for encoding subsequent frames for a given duration.
*/

#include <iostream>
#include <video_io/video_writer.hpp>

// void log_callback(const std::string& str) { std::cout << "[::video_writer::] " << str << std::endl; }

/*
static void get_frame_data(AVFrame *pict, int frame_index, int width, int height)
{
    double increment = (frame_index ) / (STREAM_DURATION * STREAM_FRAME_RATE);

    // Y plane
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            pict->data[0][y * pict->linesize[0] + x] = static_cast<uint8_t>(255 * increment);
        }
    }
 
    // Cb and Cr planes
    for (int y = 0; y < height / 2; y++)
    {
        for (int x = 0; x < width / 2; x++)
        {
            pict->data[1][y * pict->linesize[1] + x] = 127;
            pict->data[2][y * pict->linesize[2] + x] = 0;
        }
    }
}
*/

int main(int argc, char** argv)
{
	// Create video_writer object and register library callback
	vc::video_writer vw;
	// vc.set_log_callback(log_callback, vc::log_level::all);

	const auto video_path = "out.mp4";
	vw.open(video_path);
	
	// Set video info
	const auto fps = 4;
    // vw.set_fps(fps);

	const auto width = 640;
	const auto height = 480;
    // vw.set_frame_size(640, 480);

	// Write video frame by frame
	size_t num_encoded_frames = 0;
	// uint8_t* frame_data = {};

	uint8_t frame_data[352 * 288 * 3] = { 0 };

	while(num_encoded_frames < 300)
	{
        vw.write(frame_data);
        ++num_encoded_frames;
	}

	std::cout << "Decoded Frames: " << num_encoded_frames << std::endl;

    // Save local video file
    vw.save();

	// Release and cleanup video_writer
	vw.release();

	return 0;
}