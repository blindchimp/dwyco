#!/bin/bash -e
#@author Aleksandar Gotev (alex.gotev@mobimesh.it)
#Hints and code taken also from http://stackoverflow.com/questions/11929773/compiling-the-latest-openssl-for-android

if [ "$#" -ne 6 ]
then
    echo "Usage:"
    echo "./openssl-build <ANDROID_NDK_PATH> <OPENSSL_SOURCES_PATH> <ANDROID_TARGET_API> \\"
    echo "                <ANDROID_TARGET_ABI> <GCC_VERSION> <OUTPUT_PATH>"
    echo
    echo "Supported target ABIs: armeabi, armeabi-v7a, x86, x86_64, arm64-v8a, mips, mips64"
    echo
    echo "Example using GCC 4.8, NDK 10e, OpenSSL 1.0.2d and Android API 21 for armeabi-v7a."
    echo "./openssl-build /home/user/android-ndk-r10e \\"
    echo "                /home/user/openssl-1.0.2d \\"
    echo "                21 \\"
    echo "                armeabi-v7a \\"
    echo "                4.8 \\"
    echo "                /home/user/output/armeabi-v7a"
    exit 1
fi

NDK_DIR=$1
OPENSSL_BASE_FOLDER=$2
OPENSSL_TARGET_API=$3
OPENSSL_TARGET_ABI=$4
OPENSSL_GCC_VERSION=$5
OPENSSL_OUTPUT_PATH=$6
OPENSSL_TMP_FOLDER="/tmp/openssl"

#NDK_MAKE_TOOLCHAIN="$NDK_DIR/build/tools/make-standalone-toolchain.sh"
NDK_MAKE_TOOLCHAIN="$NDK_DIR/build/tools/make_standalone_toolchain.py --arch $OPENSSL_TARGET_ABI -v --api $OPENSSL_TARGET_API --stl gnustl --install-dir ${OPENSSL_TMP_FOLDER}/toolchain"
export TOOLCHAIN_PATH="${OPENSSL_TMP_FOLDER}/toolchain/bin"
rm -rf "$OPENSSL_TMP_FOLDER"
mkdir -p "$OPENSSL_TMP_FOLDER"
cp -r ${OPENSSL_BASE_FOLDER}/* ${OPENSSL_TMP_FOLDER}

function build_library {
    mkdir -p ${OPENSSL_OUTPUT_PATH}
    export PATH=$TOOLCHAIN_PATH:$PATH
    make -j 8 && make install
    #rm -rf ${OPENSSL_TMP_FOLDER}
    echo "Build completed! Check output libraries in ${OPENSSL_OUTPUT_PATH}"
}

if [ "$OPENSSL_TARGET_ABI" == "arm" ]
then
    ${NDK_MAKE_TOOLCHAIN} 
#    export TOOLCHAIN_PATH="${OPENSSL_TMP_FOLDER}/android-toolchain-arm/bin"
    export TOOL=arm-linux-androideabi
    export NDK_TOOLCHAIN_BASENAME=${TOOLCHAIN_PATH}/${TOOL}
    export CC=$NDK_TOOLCHAIN_BASENAME-gcc
    export CXX=$NDK_TOOLCHAIN_BASENAME-g++
    export LINK=${CXX}
    export LD=$NDK_TOOLCHAIN_BASENAME-ld
    export AR=$NDK_TOOLCHAIN_BASENAME-ar
    export RANLIB=$NDK_TOOLCHAIN_BASENAME-ranlib
    export STRIP=$NDK_TOOLCHAIN_BASENAME-strip
    export ARCH_FLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16"
    export ARCH_LINK="-march=armv7-a -Wl,--fix-cortex-a8"
    export CPPFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export CXXFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 -frtti -fexceptions "
    export CFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export LDFLAGS=" ${ARCH_LINK} "
    cd ${OPENSSL_TMP_FOLDER}
    ./Configure shared android-armv7 --openssldir=${OPENSSL_OUTPUT_PATH}
    build_library

elif [ "$OPENSSL_TARGET_ABI" == "arm64-v8a" ]
then
    ${NDK_MAKE_TOOLCHAIN} --platform=android-${OPENSSL_TARGET_API} \
                          --toolchain=aarch64-linux-android-4.9 \
                          --install-dir="${OPENSSL_TMP_FOLDER}/android-toolchain-arm64"
    export TOOLCHAIN_PATH="${OPENSSL_TMP_FOLDER}/android-toolchain-arm64/bin"
    export TOOL=aarch64-linux-android
    export NDK_TOOLCHAIN_BASENAME=${TOOLCHAIN_PATH}/${TOOL}
    export CC=$NDK_TOOLCHAIN_BASENAME-gcc
    export CXX=$NDK_TOOLCHAIN_BASENAME-g++
    export LINK=${CXX}
    export LD=$NDK_TOOLCHAIN_BASENAME-ld
    export AR=$NDK_TOOLCHAIN_BASENAME-ar
    export RANLIB=$NDK_TOOLCHAIN_BASENAME-ranlib
    export STRIP=$NDK_TOOLCHAIN_BASENAME-strip
    export ARCH_FLAGS=
    export ARCH_LINK=
    export CPPFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export CXXFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 -frtti -fexceptions "
    export CFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export LDFLAGS=" ${ARCH_LINK} "
    cd ${OPENSSL_TMP_FOLDER}
    ./Configure android --openssldir=${OPENSSL_OUTPUT_PATH}
    build_library

elif [ "$OPENSSL_TARGET_ABI" == "armeabi" ]
then
    ${NDK_MAKE_TOOLCHAIN} --platform=android-${OPENSSL_TARGET_API} \
                          --toolchain=arm-linux-androideabi-${OPENSSL_GCC_VERSION} \
                          --install-dir="${OPENSSL_TMP_FOLDER}/android-toolchain-arm"
    export TOOLCHAIN_PATH="${OPENSSL_TMP_FOLDER}/android-toolchain-arm/bin"
    export TOOL=arm-linux-androideabi
    export NDK_TOOLCHAIN_BASENAME=${TOOLCHAIN_PATH}/${TOOL}
    export CC=$NDK_TOOLCHAIN_BASENAME-gcc
    export CXX=$NDK_TOOLCHAIN_BASENAME-g++
    export LINK=${CXX}
    export LD=$NDK_TOOLCHAIN_BASENAME-ld
    export AR=$NDK_TOOLCHAIN_BASENAME-ar
    export RANLIB=$NDK_TOOLCHAIN_BASENAME-ranlib
    export STRIP=$NDK_TOOLCHAIN_BASENAME-strip
    export ARCH_FLAGS="-mthumb"
    export ARCH_LINK=
    export CPPFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export CXXFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 -frtti -fexceptions "
    export CFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export LDFLAGS=" ${ARCH_LINK} "
    cd ${OPENSSL_TMP_FOLDER}
    ./Configure android --openssldir=${OPENSSL_OUTPUT_PATH}
    build_library

elif [ "$OPENSSL_TARGET_ABI" == "x86" ]
then
    ${NDK_MAKE_TOOLCHAIN} 
#    export TOOLCHAIN_PATH="${OPENSSL_TMP_FOLDER}/android-toolchain-x86/bin"
    export TOOL=i686-linux-android
    export NDK_TOOLCHAIN_BASENAME=${TOOLCHAIN_PATH}/${TOOL}
    export CC=$NDK_TOOLCHAIN_BASENAME-gcc
    export CXX=$NDK_TOOLCHAIN_BASENAME-g++
    export LINK=${CXX}
    export LD=$NDK_TOOLCHAIN_BASENAME-ld
    export AR=$NDK_TOOLCHAIN_BASENAME-ar
    export RANLIB=$NDK_TOOLCHAIN_BASENAME-ranlib
    export STRIP=$NDK_TOOLCHAIN_BASENAME-strip
    export ARCH_FLAGS="-march=i686 -msse3 -mstackrealign -mfpmath=sse"
    export ARCH_LINK=
    export CPPFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export CXXFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 -frtti -fexceptions "
    export CFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export LDFLAGS=" ${ARCH_LINK} "
    cd ${OPENSSL_TMP_FOLDER}
    ./Configure shared android-x86 --openssldir=${OPENSSL_OUTPUT_PATH}
    build_library

elif [ "$OPENSSL_TARGET_ABI" == "x86_64" ]
then
    ${NDK_MAKE_TOOLCHAIN} --platform=android-${OPENSSL_TARGET_API} \
                          --toolchain=x86_64-4.9 \
                          --install-dir="${OPENSSL_TMP_FOLDER}/android-toolchain-x86_64"
    export TOOLCHAIN_PATH="${OPENSSL_TMP_FOLDER}/android-toolchain-x86_64/bin"
    export TOOL=x86_64-linux-android
    export NDK_TOOLCHAIN_BASENAME=${TOOLCHAIN_PATH}/${TOOL}
    export CC=$NDK_TOOLCHAIN_BASENAME-gcc
    export CXX=$NDK_TOOLCHAIN_BASENAME-g++
    export LINK=${CXX}
    export LD=$NDK_TOOLCHAIN_BASENAME-ld
    export AR=$NDK_TOOLCHAIN_BASENAME-ar
    export RANLIB=$NDK_TOOLCHAIN_BASENAME-ranlib
    export STRIP=$NDK_TOOLCHAIN_BASENAME-strip
    export CPPFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export CXXFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 -frtti -fexceptions "
    export CFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export LDFLAGS=" ${ARCH_LINK} "
    cd ${OPENSSL_TMP_FOLDER}
    ./Configure linux-x86_64 --openssldir=${OPENSSL_OUTPUT_PATH}
    build_library
elif [ "$OPENSSL_TARGET_ABI" == "mips" ]
then
    ${NDK_MAKE_TOOLCHAIN} --platform=android-${OPENSSL_TARGET_API} \
                          --toolchain=mipsel-linux-android-${OPENSSL_GCC_VERSION} \
                          --install-dir="${OPENSSL_TMP_FOLDER}/android-toolchain-mipsel"

    export TOOLCHAIN_PATH="${OPENSSL_TMP_FOLDER}/android-toolchain-mipsel/bin"
    export TOOL=mipsel-linux-android
    export NDK_TOOLCHAIN_BASENAME=${TOOLCHAIN_PATH}/${TOOL}
    export CC=$NDK_TOOLCHAIN_BASENAME-gcc
    export CXX=$NDK_TOOLCHAIN_BASENAME-g++
    export LINK=${CXX}
    export LD=$NDK_TOOLCHAIN_BASENAME-ld
    export AR=$NDK_TOOLCHAIN_BASENAME-ar
    export RANLIB=$NDK_TOOLCHAIN_BASENAME-ranlib
    export STRIP=$NDK_TOOLCHAIN_BASENAME-strip
    export ARCH_FLAGS=
    export ARCH_LINK=
    export CPPFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export CXXFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 -frtti -fexceptions "
    export CFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export LDFLAGS=" ${ARCH_LINK} "
    cd ${OPENSSL_TMP_FOLDER}
    ./Configure android-mips --openssldir=${OPENSSL_OUTPUT_PATH}
    build_library

elif [ "$OPENSSL_TARGET_ABI" == "mips64" ]
then
    ${NDK_MAKE_TOOLCHAIN} --platform=android-${OPENSSL_TARGET_API} \
                          --toolchain=mips64el-linux-android-4.9 \
                          --install-dir="${OPENSSL_TMP_FOLDER}/android-toolchain-mips64el"

    export TOOLCHAIN_PATH="${OPENSSL_TMP_FOLDER}/android-toolchain-mips64el/bin"
    export TOOL=mips64el-linux-android
    export NDK_TOOLCHAIN_BASENAME=${TOOLCHAIN_PATH}/${TOOL}
    export CC=$NDK_TOOLCHAIN_BASENAME-gcc
    export CXX=$NDK_TOOLCHAIN_BASENAME-g++
    export LINK=${CXX}
    export LD=$NDK_TOOLCHAIN_BASENAME-ld
    export AR=$NDK_TOOLCHAIN_BASENAME-ar
    export RANLIB=$NDK_TOOLCHAIN_BASENAME-ranlib
    export STRIP=$NDK_TOOLCHAIN_BASENAME-strip
    export ARCH_FLAGS=
    export ARCH_LINK=
    export CPPFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export CXXFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 -frtti -fexceptions "
    export CFLAGS=" ${ARCH_FLAGS} -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 "
    export LDFLAGS=" ${ARCH_LINK} "
    cd ${OPENSSL_TMP_FOLDER}
    ./Configure android-mips64 --openssldir=${OPENSSL_OUTPUT_PATH}
    build_library

else
    echo "Unsupported target ABI: $OPENSSL_TARGET_ABI"
    exit 1
fi
