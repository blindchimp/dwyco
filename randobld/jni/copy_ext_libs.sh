#
# this just copies the built external libs into the places
# where the android project expects to find them
#
cp -v \
 ./libogg/output/lib/libogg.a \
./libspeex/output/lib/libspeex.a \
./libspeex/output/lib/libspeexdsp.a \
./libtheora/output/lib/libtheora.a \
./libtheora/output/lib/libtheoradec.a \
./libtheora/output/lib/libtheoraenc.a \
./libvorbis/output/lib/libvorbis.a \
./libvorbis/output/lib/libvorbisenc.a \
./libvorbis/output/lib/libvorbisfile.a \
./jpeg/output/lib/libjpeg.a \
../obj/local/armeabi-v7a
