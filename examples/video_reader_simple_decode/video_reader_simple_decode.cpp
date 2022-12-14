/**
 * example: 	video_capture_simple_decode
 * author:		Stefano Lusardi
 * date:		Jun 2021
 * description:	The simplest example to show video_reader API usage. 
 * 				Single threaded: main thread is responsible for decode subsequent frames until the video ends.
 * 				In case an rtsp stream is provided the main loop will never return.
*/

#include <iostream>
#include <video_io/video_reader.hpp>

void log_callback(const std::string& str) { std::cout << "[::video_reader::] " << str << std::endl; }

int main(int argc, char** argv)
{
	// Create video_reader object and register library callback
	vio::video_reader v;
	// v.set_log_callback(log_callback, vio::log_level::all);

	// Open video (local file, RTSP, ...)
	const auto video_path = "../../../../tests/data/testsrc_10sec_30fps.mkv";
	// const auto video_path = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_175k.mov";
	v.open(video_path);//, vio::decode_support::HW);
	
	// Retrieve video info
	const auto fps = v.get_fps();
	const auto size = v.get_frame_size();
	const auto [width, height] = size.value();

	// Read video frame by frame
	size_t num_decoded_frames = 0;
	uint8_t* frame_data = {};
	while(v.read(&frame_data))
	{
		++num_decoded_frames;
		// Use frame_data array
		// ...
	}
	
	std::cout << "Decoded Frames: " << num_decoded_frames << std::endl;

	// Release and cleanup video_reader
	v.release();

	return 0;
}