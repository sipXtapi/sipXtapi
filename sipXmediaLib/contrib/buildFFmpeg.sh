#!/bin/bash

echo "WARNING: not all of the ffmpeg project is LGPL."
echo "This configuration was created to build the AAC codec dependeices using only LGPL FFMpeg."
echo "Please do not change the configuration without confirming that only LGPL code is used."
echo
echo "WARNING: the AAC codec requires intelectual property licensing, be sure you use proper licensing."
echo

TARBALL="http://ffmpeg.org/releases/ffmpeg-0.8.tar.gz"
VERSION_DIR="ffmpeg-0.8"

if [ -d ffmpeg ]
then
    echo "Directory ffmpeg already exists."
else
    echo "Downloading FFmpeg tarball: ${TARBALL}"
    wget ${TARBALL}
    tar -zxvf `basename ${TARBALL}`
    mv ${VERSION_DIR}  ffmpeg
fi

if [ ! -d ffmpeg ]
then
    echo "Unable to get FFmpeg tarball: ${TARBALL}"
    exit 1
fi

cd ffmpeg

./configure --disable-ffplay --disable-ffprobe --disable-ffserver --disable-avdevice --disable-swscale --disable-postproc --disable-avfilter --disable-swscale-alpha --disable-protocols --disable-devices --disable-filters --disable-muxers --enable-muxer=adts --disable-demuxers --enable-demuxer=aac --disable-bsfs --enable-bsf=aac_adtstoasc --disable-parsers --enable-parser=aac --enable-parser=mpegaudio --disable-encoders --disable-decoders --enable-decoder=aac --enable-encoder=aac --extra-cflags=-fPIC --enable-shared

make

