FROM ubuntu:22.04 AS base

ARG C_COMPILER=gcc
ARG CXX_COMPILER=g++
ARG COMPILER_VERSION=10
ARG BUILD_TYPE=Release

RUN apt-get update -qq && export DEBIAN_FRONTEND=noninteractive && \
    apt-get install -y --no-install-recommends \
    make cmake ninja-build \
    python3.10 python3-pip \
    pkg-config \
    ${C_COMPILER}-${COMPILER_VERSION} \
    ${CXX_COMPILER}-${COMPILER_VERSION}

ENV CC=${C_COMPILER}-${COMPILER_VERSION}
ENV CXX=${CXX_COMPILER}-${COMPILER_VERSION}

RUN python3 -m pip install --upgrade pip setuptools && \
    python3 -m pip install conan && \
    conan profile new default --detect && \
    conan profile update settings.compiler=${C_COMPILER} default && \
    conan profile update settings.compiler.version=${COMPILER_VERSION} default && \
    conan profile update settings.compiler.libcxx=libstdc++11 default

ENV CONAN_SYSREQUIRES_SUDO 0
ENV CONAN_SYSREQUIRES_MODE enabled

WORKDIR /video_io

COPY /conanfile.txt conanfile.txt
RUN  conan install . \
    --install-folder build/${BUILD_TYPE}/modules \
    --settings build_type=${BUILD_TYPE} \
    --settings compiler=${C_COMPILER} \
    --settings compiler.version=${COMPILER_VERSION} \
    --build missing \
    -c tools.system.package_manager:mode=install

FROM base AS test
ARG BUILD_TYPE=Release
COPY /tests/conanfile.txt tests/conanfile.txt
RUN  conan install tests \
    --install-folder build/${BUILD_TYPE}/modules \
    --settings build_type=${BUILD_TYPE} \
    --settings compiler=${C_COMPILER} \
    --settings compiler.version=${COMPILER_VERSION} \
    --build missing \
    -c tools.system.package_manager:mode=install

COPY . .
RUN cmake -G Ninja -B ./build/${BUILD_TYPE} -S . \ 
  -D CMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -D VIDEO_IO_BUILD_TESTS=ON \
  -D VIDEO_IO_BUILD_EXAMPLES=OFF \
  -D VIDEO_IO_BUILD_BENCHMARKS=OFF \
  -D VIDEO_IO_BUILD_SANITIZERS=OFF \
  -D VIDEO_IO_BUILD_DOCS=OFF
RUN cmake --build ./build/${BUILD_TYPE} --config ${BUILD_TYPE}
RUN ctest --test-dir ./build/${BUILD_TYPE} --config ${BUILD_TYPE} --output-junit results.xml --output-on-failure -j$(nproc)

FROM base AS build
ARG BUILD_TYPE=Release

COPY examples/conanfile.txt examples/conanfile.txt
RUN  conan install examples \
    --install-folder build/${BUILD_TYPE}/modules \
    --settings build_type=${BUILD_TYPE} \
    --settings compiler=${C_COMPILER} \
    --settings compiler.version=${COMPILER_VERSION} \
    --build missing \
    -c tools.system.package_manager:mode=install

COPY . .
RUN cmake -G Ninja -B ./build/${BUILD_TYPE} -S . \ 
  -D CMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -D VIDEO_IO_BUILD_TESTS=OFF \
  -D VIDEO_IO_BUILD_EXAMPLES=ON \
  -D VIDEO_IO_BUILD_BENCHMARKS=OFF \
  -D VIDEO_IO_BUILD_SANITIZERS=OFF \
  -D VIDEO_IO_BUILD_DOCS=OFF
RUN cmake --build ./build/${BUILD_TYPE} --config ${BUILD_TYPE}
RUN cmake --install ./build/${BUILD_TYPE} --prefix ./install

FROM ubuntu:22.04 AS publish
COPY --from=build /video_io/install /video_io
# CMD [ "./video_io/app" ]

# FROM scratch AS publish
# COPY --from=base /lib/x86_64-linux-gnu/libpthread.so.0 /lib/x86_64-linux-gnu/libpthread.so.0
# COPY --from=base /lib/x86_64-linux-gnu/libstdc++.so.6 /lib/x86_64-linux-gnu/libstdc++.so.6
# COPY --from=base /lib/x86_64-linux-gnu/libgcc_s.so.1 /lib/x86_64-linux-gnu/libgcc_s.so.1
# COPY --from=base /lib/x86_64-linux-gnu/libc.so.6 /lib/x86_64-linux-gnu/libc.so.6
# COPY --from=base /lib64/ld-linux-x86-64.so.2 /lib64/ld-linux-x86-64.so.2
# COPY --from=base /lib/x86_64-linux-gnu/libm.so.6 /lib/x86_64-linux-gnu/libm.so.6
# COPY --from=build /docker_cpp/install /docker_cpp
# CMD [ "./docker_cpp/app" ]
