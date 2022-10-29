/**
 * example: 	video_player_opencv_multi_thread
 * author:		Stefano Lusardi
 * date:		Jun 2021
 * description:	Example to show how to integrate vio::video_reader in a simple video player based on OpenCV framework. 
 * 				Multi threaded: one (background) thread decodes and enqueue frames, the other (main) dequeues and renders them in order.
 * 				For simpler examples(single thread) you might want to have a look at any video_player_xxx example (no multi_thread).
*/

#include <iostream>
#include <thread>

#include <video_io/video_reader.hpp> 
#include <opencv2/highgui.hpp>

void log_info(const std::string& str)  { std::cout << "[  INFO ] " << str << std::endl; }
void log_error(const std::string& str) { std::cout << "[ ERROR ] " << str << std::endl; }

int main(int argc, char** argv)
{
	std::string video_path;
	if (argc < 2)
	{
		std::cout << "Missing video file path." << std::endl;
		std::cout << "Usage: \nvideo_player_opencv <VIDEO_PATH>" << std::endl;
		// video_path = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
		video_path = "../../../../tests/data/testsrc_10sec_30fps.mkv";
		std::cout << "Using default RTSP video stream: " << video_path << std::endl;
	}
	else
	{
		video_path = argv[1];
		std::cout << "Using video file: " << video_path << std::endl;
	}

	vio::video_reader v;
	// v.set_log_callback(log_info, vio::log_level::info);
	// v.set_log_callback(log_error, vio::log_level::error);

	if(!v.open(video_path, vio::decode_support::HW))
	{
		std::cout << "Unable to open " << video_path << std::endl;
		return -1;
	}

	const auto size = v.get_frame_size(); 
	if(!size)
	{
		std::cout << "Unable to retrieve frame size from " << video_path << std::endl;
		return -1;
	}

	const auto fps = v.get_fps();
	if(!fps)
	{
		std::cout << "Unable to retrieve FPS from " << video_path << std::endl;
		return -1;
	}

	// NOTE: height is the number of cv::Mat rows, width is the number of cv::Mat cols.
	const auto [w, h] = size.value();
	cv::Mat frame(h, w, CV_8UC3); 

	const auto frame_time = std::chrono::nanoseconds(static_cast<int>(1'000'000'000/fps.value()));
	
	const std::string window_title = "vio::video_reader";
	cv::namedWindow(window_title);

	int n_frames = 0;
	auto total_start_time = std::chrono::high_resolution_clock::now();

	while(true)
	{
		if(!v.read(&frame.data))
			break;

		cv::imshow(window_title, frame);
		cv::waitKey(1);
		++n_frames;
	}

	auto total_end_time = std::chrono::high_resolution_clock::now();
	std::cout << "Decode time: " << std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count() << "ms" << std::endl;
	std::cout << "Decoded Frames: " << n_frames << std::endl;

	v.release();
	cv::destroyAllWindows();
	
	return 0;
}