# video_io

video encoder and decoder, written in modern C++

---

## Prerequisites
- C++17 compliant compiler
- CMake
- Python3
- Ninja (optional)

## Supported configurations
- Ubuntu 20.04: GCC 9
- Ubuntu 22.04: GCC 10
- Windows 10: MSVC 17 (Visual Studio 2022)
- Windows 11: MSVC 17 (Visual Studio 2022)

## Setup Conan
```bash
pip install conan
```

## Configure Dependencies
```bash
# Linux/Mac
./scripts/conan_install.sh Debug
./scripts/conan_install.sh Release

# Windows
scripts\conan_install.bat Debug
scripts\conan_install.bat Release
```

## Configure Project
```bash
cmake -G Ninja -D CMAKE_BUILD_TYPE=Debug -B ./build/Debug -S .
cmake -G Ninja -D CMAKE_BUILD_TYPE=Release -B ./build/Release -S .
```

## Build Project
```bash
cmake --build ./build/Debug --config Debug
cmake --build ./build/Release --config Release
```

---

## Tests

## Examples

## Docs

## Benchmarks

## CI