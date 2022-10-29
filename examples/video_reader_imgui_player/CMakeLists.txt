set(TARGET_NAME video_reader_imgui_player)

find_package(imgui)
find_package(glfw3)

add_executable(${TARGET_NAME}
	video_reader_imgui_player.cpp
	${CMAKE_BINARY_DIR}/bindings/imgui_impl_glfw.cpp
	${CMAKE_BINARY_DIR}/bindings/imgui_impl_glfw.h
	${CMAKE_BINARY_DIR}/bindings/imgui_impl_opengl3.cpp
	${CMAKE_BINARY_DIR}/bindings/imgui_impl_opengl3.h
)

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR}/bindings)

target_link_libraries(${TARGET_NAME}
	PRIVATE vio::video_io-static
	PRIVATE imgui::imgui
	PRIVATE glfw
)