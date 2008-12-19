#!/bin/sh

# Copyright (C) 2008 SIPez LLC. 
#
# Script for building sipxtapi doxygen documentation
#
# Based off a script from Sergey Konstanbaev <sergey.konstanbaev@sipez.com>
# Extended and optimized for continuous-integration build systems 
# (specifically Hudson) by Keith Kyzivat <kkyzivat@sipez.com>


if [ ! -e sipXportLib ] ; then 
  echo "You don't appear to be in a sipxtapi working dir."
  echo "Please change your current directory to a sipxtapi working directory"
  echo "and re-run this script."
  exit 2
fi

# Figure out where to install things to... Make a temporary directory for it.
# For docs, this shouldn't contain anything, and we'll remove it at the end.
# so no need to tell the user where it is.
INSTALLDIR=`mktemp -t -d sipxtapi_stage.XXXXXX`
exitStatus=$?
if [ $exitStatus -gt 0 ] ; then
  # Failed to create temporary directory for installing to, for some reason.
  # exit failure.
  echo "Failed to create temporary directory, for some reason ($exitStatus)" 
  exit 3
fi

echo "Installing to directory: $INSTALLDIR"

# Define where we're storing the documentation zipfile artifact
DOCZIP_PATH="$PWD/sipxtapi_doc.zip"


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

rm -rf $INSTALLDIR &&

(
  cd sipXportLib &&
  mkdir -p build &&
  autoreconf -fi &&
  cd build &&
  ../configure --prefix=$INSTALLDIR ${CONFIGFLAGS} &&
  make doc &&
  make -C doc install-doc
) &&

(
  cd sipXsdpLib &&
  sed -e '0,/SFAC_LIB_/s/SFAC_LIB_[^ ]*/SFAC_INIT_FLAGS/;/SFAC_LIB_/d' configure.ac > configure.ac.nolibreqs &&
  mv configure.ac.nolibreqs configure.ac &&
  mkdir -p build &&
  autoreconf -fi &&
  cd build &&
  ../configure --prefix=$INSTALLDIR ${CONFIGFLAGS} &&
  make doc &&
  make -C doc install-doc
) &&

(
  cd sipXtackLib &&
  sed -e '0,/SFAC_LIB_/s/SFAC_LIB_[^ ]*/SFAC_INIT_FLAGS/;/SFAC_LIB_/d' configure.ac > configure.ac.nolibreqs &&
  mv configure.ac.nolibreqs configure.ac &&
  mkdir -p build &&
  autoreconf -fi &&
  cd build &&
  ../configure --prefix=$INSTALLDIR ${CONFIGFLAGS} --disable-sipviewer &&
  make doc &&
  make -C doc install-doc
) &&

(
  cd sipXmediaLib &&
  sed -e '0,/SFAC_LIB_/s/SFAC_LIB_[^ ]*/SFAC_INIT_FLAGS/;/SFAC_LIB_/d' configure.ac > configure.ac.nolibreqs &&
  mv configure.ac.nolibreqs configure.ac &&
  mkdir -p build &&
  autoreconf -fi &&
  cd build &&
  ../configure --prefix=$INSTALLDIR  ${CONFIGFLAGS} &&
  make doc &&
  make -C doc install-doc
) &&

(
  cd sipXmediaAdapterLib &&
  sed -e '0,/SFAC_LIB_/s/SFAC_LIB_[^ ]*/SFAC_INIT_FLAGS/;/SFAC_LIB_/d' configure.ac > configure.ac.nolibreqs &&
  mv configure.ac.nolibreqs configure.ac &&
  mkdir -p build &&
  autoreconf -fi &&
  cd build &&
  ../configure --prefix=$INSTALLDIR ${CONFIGFLAGS} &&
  make doc &&
  make -C doc install-doc
) &&

(
  cd sipXcallLib &&
  sed -e '0,/SFAC_LIB_/s/SFAC_LIB_[^ ]*/SFAC_INIT_FLAGS/;/SFAC_LIB_/d' configure.ac > configure.ac.nolibreqs &&
  mv configure.ac.nolibreqs configure.ac &&
  mkdir -p build &&
  autoreconf -fi &&
  cd build &&
  ../configure --prefix=$INSTALLDIR ${CONFIGFLAGS} &&
  make doc &&
  make -C doc install-doc
) &&

# Zip up the documentation.  We have to delve into each directory so that the
# paths encoded in the zip file are correct.
(
  cd $INSTALLDIR/share &&
  zip -qr $DOCZIP_PATH doc
) &&

echo "Installdir: $INSTALLDIR" && 
echo "Successfully built sipXtapi documentation" &&
echo "$DOCZIP_PATH"
