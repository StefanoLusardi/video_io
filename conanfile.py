#!/usr/bin/env python
from conans import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps

class VideoIoConan(ConanFile):
    name = "video_io"
    version = "0.1.0"
    license = "MIT"
    author = "Stefano Lusardi lusardi.stefano@gmail.com"
    url = "https://github.com/StefanoLusardi/video_io"
    description = "video encoder and decoder, written in modern C++ "
    topics = ("video", "encoding", "decoding")
    generators = "cmake_find_package"
    exports_sources = ["CMakeLists.txt", "LICENSE", "VERSION", "video_io/*"]
    exports = ["LICENSE"]
    no_copy_source = True
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        
        self.options["ffmpeg"].disable_everything=True
        self.options["ffmpeg"].with_asm=True
        self.options["ffmpeg"].with_zlib=True
        self.options["ffmpeg"].with_bzip2=True
        self.options["ffmpeg"].with_lzma=False
        self.options["ffmpeg"].with_libiconv=False
        self.options["ffmpeg"].with_freetype=False
        self.options["ffmpeg"].with_openjpeg=False
        self.options["ffmpeg"].with_openh264=True
        self.options["ffmpeg"].with_opus=False
        self.options["ffmpeg"].with_vorbis=False
        self.options["ffmpeg"].with_zeromq=False
        self.options["ffmpeg"].with_sdl=False
        self.options["ffmpeg"].with_libx264=True
        self.options["ffmpeg"].with_libx265=True
        self.options["ffmpeg"].with_libvpx=False
        self.options["ffmpeg"].with_libmp3lame=False
        self.options["ffmpeg"].with_libfdk_aac=False
        self.options["ffmpeg"].with_libwebp=False
        self.options["ffmpeg"].with_ssl="openssl"
        self.options["ffmpeg"].with_libalsa=False
        self.options["ffmpeg"].with_pulse=False
        self.options["ffmpeg"].with_vaapi=False
        self.options["ffmpeg"].with_vdpau=False
        self.options["ffmpeg"].with_vulkan=False
        self.options["ffmpeg"].with_xcb=False
        self.options["ffmpeg"].with_programs=False

    def requirements(self):
        self.requires("ffmpeg/5.1")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["VIDEO_IO_BUILD_TESTS"] = False
        tc.variables["VIDEO_IO_BUILD_EXAMPLES"] = False
        tc.variables["VIDEO_IO_BUILD_SANITIZERS"] = False
        tc.variables["VIDEO_IO_BUILD_BENCHMARKS"] = False
        tc.variables["VIDEO_IO_BUILD_DOCS"] = False
        tc.variables["VIDEO_IO_INTERNAL_LOGGER"] = False
        
        tc.generate()
        cmake_deps = CMakeDeps(self)
        cmake_deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy(pattern="LICENSE")
        self.copy(pattern="VERSION")
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["video_io"]
        # self.cpp_info.filenames["cmake_find_package"] = "video_io"

