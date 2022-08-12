# video_io

video encoder and decoder, written in modern C++

---

## Prerequisites
- CMake
- Python3
- Ninja
- MSVC (on Windows), GCC or Clang (on Linux/Mac)

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