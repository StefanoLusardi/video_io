/**
 * example: 	video_player_imgui
 * author:		Stefano Lusardi
 * date:		Jun 2022
 * description:	Example to show how to integrate vio::video_reader in a simple video player based on OpenGL (using GLFW). 
 * 				Single threaded: Main thread decodes and draws subsequent frames.
 * 				Note that this serves only as an example, as in real world application 
 * 				you might want to handle decoding and rendering on separate threads (see any video_player_xxx_multi_thread).
*/

#include <memory>
#include <iostream>
#include <thread>
#include <atomic>

#include <video_io/video_reader.hpp>
#include "../utils/simple_frame.hpp"
#include "../utils/frame_queue.hpp"

#include <imgui.h>
#include <GLFW/glfw3.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using namespace std::chrono_literals;


double get_elapsed_time()
{
	static std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>> start_time = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_time = std::chrono::steady_clock::now() - start_time;
	return elapsed_time.count();
}

int main(int argc, char **argv)
{
	std::cout << "GLFW version: " << glfwGetVersionString() << std::endl;
	vio::video_reader v;
	// const auto video_path = "../../../tests/data/testsrc_120sec_30fps.mkv";
	const auto video_path = "../../../tests/data/testsrc_10sec_30fps.mkv";

	if (!v.open(video_path, vio::decode_support::SW))
	{
		std::cout << "Unable to open video: " << video_path << std::endl;
		return 1;
	}

	const auto fps = v.get_fps();
	const auto size = v.get_frame_size();
	const auto [frame_width, frame_height] = size.value();

	if (!glfwInit())
	{
		std::cout << "Couldn't init GLFW" << std::endl;
		return 1;
	}
	
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow *window = glfwCreateWindow(frame_width, frame_height, "Video Player OpenGL", NULL, NULL);
	if (!window)
	{
		std::cout << "Couldn't open window" << std::endl;
		return 1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

   	int screen_width, screen_height;
	glfwGetFramebufferSize(window, &screen_width, &screen_height);
	glViewport(0, 0, screen_width, screen_height);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGui::StyleColorsDark();

	GLuint tex_handle;
	glGenTextures(1, &tex_handle);
	glBindTexture(GL_TEXTURE_2D, tex_handle);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	// glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>> start_time = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_time(0.0);
	
	auto total_start_time = std::chrono::high_resolution_clock::now();
	auto total_end_time = std::chrono::high_resolution_clock::now();

	// int window_width, window_height;
	// glfwGetFramebufferSize(window, &window_width, &window_height);
	// glMatrixMode(GL_PROJECTION);
	// glLoadIdentity();
	// glOrtho(0, window_width, window_height, 0, -1, 1);
	// glMatrixMode(GL_MODELVIEW);

	std::unique_ptr<vio::simple_frame> frame;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		uint8_t* frame_data = {};
		if (!v.read(&frame_data))
		{
			total_end_time = std::chrono::high_resolution_clock::now();
			std::cout << "Couldn't load video frame" << std::endl;
			std::cout << "Video finished" << std::endl;
			break;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame_data);
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame->data.data());

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex_handle);
		glBegin(GL_QUADS);

		glTexCoord2d(0, 0);
		glVertex2i(0, 0);

		glTexCoord2d(1, 0);
		glVertex2i(frame_width, 0);

		glTexCoord2d(1, 1);
		glVertex2i(frame_width, frame_height);

		glTexCoord2d(0, 1);
		glVertex2i(0, frame_height);

		glEnd();
		glDisable(GL_TEXTURE_2D);

		ImGui::Begin("OpenGL Texture Text");
		ImGui::Text("pointer = %d", tex_handle);
		ImGui::Text("size = %d x %d", screen_width, screen_height);
		ImGui::Image((void*)(intptr_t)tex_handle, ImVec2(screen_width, screen_height));
		ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

	std::cout << "Decode time: " << std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count() << "ms" << std::endl;
	
	v.release();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}