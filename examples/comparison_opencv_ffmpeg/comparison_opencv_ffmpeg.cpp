/**
 * example: 	simple_decode
 * author:		Stefano Lusardi
 * date:		Jun 2021
 * description:	Comparison between OpenCV::VideoCapture and vio::video_reader. 
 * 				Public APIs are very similar, while private ones are simplified quite a lot.
 * 				The frames are using OpenCV::imshow in both examples.
 * 				Note that sleep time between consecutive frames is not accurate here, 
 * 				see any video_player_xxx example for a more accurate playback.
*/

#include <memory>
#include <iostream>
#include <thread>

#include <video_io/video_reader.hpp>
// #include <video_reader/raw_frame.hpp>

#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

void run_opencv(const char* video_path, const char* name)
{
	cv::VideoCapture v;
	if(!v.open(video_path))
	{
		std::cout << "Unable to open " << video_path << std::endl;
		return;
	}

	const auto w = v.get(cv::CAP_PROP_FRAME_WIDTH);
	const auto h = v.get(cv::CAP_PROP_FRAME_HEIGHT);
	const auto fps = v.get(cv::CAP_PROP_FPS);
	const auto sleep_time = 1;
	cv::Mat frame(w, h, CV_8UC3);

	cv::namedWindow(name);

	auto start = std::chrono::steady_clock::now();
	while(v.read(frame))
	{
		cv::imshow(name, frame);
		cv::waitKey(sleep_time);
	}
	auto end = std::chrono::steady_clock::now();

	std::cout << name << " - " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << std::endl;

	v.release();
	cv::destroyWindow(name);
}

void run_video_io(const char* video_path, const char* name, vio::decode_support decode_support)
{
	vio::video_reader v;
	if(!v.open(video_path, decode_support))
	{
		std::cout << "Unable to open " << video_path << std::endl;
		return;
	}

	const auto size = v.get_frame_size(); 
	const auto [w, h] = size.value();
	const auto fps = v.get_fps();
	const auto sleep_time = 1;
	cv::Mat frame(h, w, CV_8UC3);

	cv::namedWindow(name);

	auto start = std::chrono::steady_clock::now();
	while(v.read(&frame.data))
	{
		cv::imshow(name, frame);
		cv::waitKey(sleep_time);
	}
	auto end = std::chrono::steady_clock::now();
	
	std::cout << name << " - " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << std::endl;

	v.release();
	cv::destroyWindow(name);
}

int main(int argc, char** argv)
{
	const auto video_path = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mp4";
	// const auto video_path = "../../../../tests/data/testsrc_120sec_30fps.mpg";
	
	std::thread opencv_thread([video_path]{ run_opencv(video_path, "OpenCV"); });
	std::thread video_io_thread_sw([video_path]{ run_video_io(video_path, "video_io SW", vio::decode_support::SW); });
	std::thread video_io_thread_hw([video_path]{ run_video_io(video_path, "video_io HW", vio::decode_support::HW); });

	if(opencv_thread.joinable())
		opencv_thread.join();

	if(video_io_thread_sw.joinable())
		video_io_thread_sw.join();

	if(video_io_thread_hw.joinable())
		video_io_thread_hw.join();

	return 0;
}