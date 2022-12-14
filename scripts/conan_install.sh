BUILD_TYPE=$1
echo BUILD_TYPE: $BUILD_TYPE

COMPILER=$2
echo COMPILER: $COMPILER

COMPILER_VERSION=$3
echo COMPILER_VERSION: $COMPILER_VERSION

export CONAN_SYSREQUIRES_SUDO=0
export CONAN_SYSREQUIRES_MODE=enabled
export DEBIAN_FRONTEND=noninteractive

conan install video_io \
    --install-folder build/$BUILD_TYPE/modules/video_io \
    --settings build_type=$BUILD_TYPE \
    --settings compiler=$COMPILER \
    --settings compiler.version=$COMPILER_VERSION \
    --build missing

conan install tests \
    --install-folder build/$BUILD_TYPE/modules/tests \
    --settings build_type=$BUILD_TYPE \
    --settings compiler=$COMPILER \
    --settings compiler.version=$COMPILER_VERSION \
    --build missing

conan install examples \
    --install-folder build/$BUILD_TYPE/modules/examples \
    --settings build_type=$BUILD_TYPE \
    --settings compiler=$COMPILER \
    --settings compiler.version=$COMPILER_VERSION \
    --build missing

conan install benchmarks \
    --install-folder build/$BUILD_TYPE/modules/benchmarks \
    --settings build_type=$BUILD_TYPE \
    --settings compiler=$COMPILER \
    --settings compiler.version=$COMPILER_VERSION \
    --build missing
