#!/bin/sh

# Copyright (C) 2008 SIPez LLC. 
#
# Script for building sipxtapi from ground up.
# Currently this builds ground-up to sipXmediaLib, however this 
# should be extended to build everything including sipXtackLib
# sipXcallLib, and sipXtapi
#
# Based off a script from Sergey Konstanbaev <sergey.konstanbaev@sipez.com>
# Extended and optimized for continuous-integration build systems 
# (specifically Hudson) by Keith Kyzivat <kkyzivat@sipez.com>

# Right now this script only builds up through sipXmediaAdapterLib

# when building codec_speex plugin, you need to pass -fPIC to compiler.
# Also, speex cannot deal with non-linear build, so you need to pass -j1 
# when building sipXmediaLib below.
export CFLAGS="-O2 -g -fPIC"
export CXXFLAGS=$CFLAGS

if [ ! -e sipXportLib ] ; then 
  echo "You don't appear to be in a sipxtapi working dir."
  echo "Please change your current directory to a sipxtapi working directory"
  echo "and re-run this script."
  exit 2
fi

# Apply patch to sipXmediaLib so that autoconf 2.59 will work.
(
  exitStatus=0
  cd sipXmediaLib
  ac259patchapplied=`svn stat | grep "contrib/libspeex/doc/Makefile.am" | wc -l`
  if [ $ac259patchapplied -eq 0 ] ; then 
    patch -p0 < autoconf259.patch
    if [ $? -gt 0 ] ; then
      # Exit non-zero so that automated build will detect failure properly
      exitStatus=2
    fi
  fi
  exit $exitStatus
) &&

rm -rf /tmp/stage &&

(
  cd sipXportLib &&
  mkdir -p build &&
  autoreconf -fi &&
  cd build &&
  ../configure --prefix=/tmp/stage ${CONFIGFLAGS} &&
  make -j3 && make install
) &&

(
  cd sipXsdpLib &&
  mkdir -p build &&
  autoreconf -fi &&
  cd build &&
  ../configure --prefix=/tmp/stage ${CONFIGFLAGS} &&
  make -j3 && make install
) &&

(
  cd sipXtackLib/include &&
  cp -rp net /tmp/stage/include
) &&

(
  cd sipXcallLib/include &&
  cp -rp tapi /tmp/stage/include
) &&

(
  cd sipXmediaLib &&
  mkdir -p build &&
  autoreconf -fi &&
  cd build &&
  ../configure --prefix=/tmp/stage --enable-local-audio --enable-speex-dsp --enable-codec-speex --disable-stream-player ${CONFIGFLAGS} &&
  make -j1 && make install &&
  cd contrib/libspeex && make install
) &&

(
  cd sipXmediaAdapterLib &&
  mkdir -p build &&
  autoreconf -fi &&
  cd build &&
  ../configure --prefix=/tmp/stage --enable-topology-graph --disable-stream-player ${CONFIGFLAGS} &&
  make -j3 && make install
)

