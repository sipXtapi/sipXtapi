#/bin/bash

# Copyright 2008 AOL LLC.
# Licensed to SIPfoundry under a Contributor Agreement.
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
# USA. 
# 
# Copyright (C) 2004-2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
# Licensed to SIPfoundry under a Contributor Agreement.

PREFIX=`pwd`/dist.`uname`
PREFIX_ARCHIVE=`pwd`
BUILD_ROOT=`pwd`/..
SVN_VERSION=`svn info ${BUILD_ROOT} | grep Revision | cut -b 11-`
SVN_URL=`svn info ${BUILD_ROOT} | grep URL | cut -b 6-`
REBUILD=0
MP_SIPFOUNDRY=1
MP_GIPS=0

# Process command line arguments
while [ $# -gt 0 ]
do
  if [ "$1" == "-r" ]
  then
    REBUILD=1
  elif [ $1 == "-gips" ]
  then
    MP_GIPS=1
    MP_SIPFOUNDRY=0
  else
    echo "Unknown command line argument: " $1
    exit 1
  fi
  shift
done

echo ""
echo ----------------------------------------
echo "Build Root :" ${BUILD_ROOT}
echo "Build Stage:" ${PREFIX}
echo "Version    :" ${SVN_VERSION}
echo "URL        :" ${SVN_URL}
if [ $REBUILD == 1 ]
then
  echo "Build Type : Rebuild"
else
  echo "Build Type : Full Build"
fi
if [ $MP_SIPFOUNDRY == 1 ]
then
  echo "MP Library : SIP Foundry"
else
  echo "MP Library : GIPS VEMM"
fi
echo ----------------------------------------
echo ""

function updateVersion
{
  echo ""
  echo ----------------------------------------
  echo Setting Version: ${SVN_VERSION}
  echo ----------------------------------------

  sed -i -e "s/SIPXTAPI_BUILDNUMBER.*\".*\"/SIPXTAPI_BUILDNUMBER \"${SVN_VERSION}\"/g" \
      sipXcallLib/include/tapi/sipXtapi.h
  sed -i -e "s/SIPXTAPI_BUILD_WORD [0-9,,]*/SIPXTAPI_BUILD_WORD 2,10,1,${SVN_VERSION}/g" \
      sipXcallLib/include/tapi/sipXtapi.h
  sed -i -e "s/SIPXTAPI_FULL_VERSION.*\".*\"/SIPXTAPI_FULL_VERSION \"2.10.1.${SVN_VERSION}\"/g" \
      sipXcallLib/include/tapi/sipXtapi.h
  sed -i -e "s/SIPXTAPI_BUILDDATE.*\".*\"/SIPXTAPI_BUILDDATE \"`date +%F`\"/g" \
      sipXcallLib/include/tapi/sipXtapi.h

  echo Done
  echo ""
}

function revertVersion
{
  echo ""
  echo ----------------------------------------
  echo Reverting Version
  echo ----------------------------------------

  svn revert sipXcallLib/include/tapi/sipXtapi.h
  echo Done
  echo ""
}

function cleanupAndExit()
{
    revertVersion
    echo Exiting rc: $1 
    exit $1
}

function build
{
  echo ""
  echo ----------------------------------------
  echo Building $1
  echo ----------------------------------------

  pushd $1 > /dev/null

  if [ $REBUILD != 1 ]
  then
    autoreconf -fi
    if [ $? -ne 0 ]
    then
      popd > /dev/null
      cleanupAndExit 1
    fi
    ./configure --prefix=$2 $3 $4 $5
    if [ $? -ne 0 ]
    then
      popd > /dev/null
      cleanupAndExit 1
    fi
    make clean
    if [ $? -ne 0 ]
    then
      popd > /dev/null
      cleanupAndExit 1
    fi
  fi

  make 
  if [ $? -ne 0 ]
  then
    popd > /dev/null
    cleanupAndExit 1
  fi
  make install
  if [ $? -ne 0 ]
  then
    popd > /dev/null
    cleanupAndExit 1
  fi
  popd > /dev/null

  echo Done
  echo ""
}

function package_source
{
  echo ""
  echo ----------------------------------------
  echo Exporting Source
  echo ----------------------------------------
  pushd ${PREFIX_ARCHIVE} > /dev/null
  TARGET_NAME=sipXtapi_SRC_r${SVN_VERSION}_`date +%F`
  TARGET_DIR=${TARGET_NAME}
  rm -rf ${TARGET_DIR} > /dev/null
  svn export ${SVN_URL} ${TARGET_DIR}
  echo Done
  echo ""

  echo ""
  echo ----------------------------------------
  echo Packaging Source
  echo ----------------------------------------
  tar -czf ${TARGET_NAME}.tgz ${TARGET_DIR}
  popd > /dev/null
  echo Done
  echo ""
}

function package_libs
{
  echo ""
  echo ----------------------------------------
  echo Packaging Binaries
  echo ----------------------------------------

  pushd ${PREFIX_ARCHIVE} > /dev/null
  TARGET_NAME=sipXtapi_linux_r${SVN_VERSION}_`date +%F`
  TARGET_DIR=${TARGET_NAME}
  rm -rf ${TARGET_DIR} > /dev/null
  mkdir ${TARGET_DIR}
  mkdir ${TARGET_DIR}/lib
  cp ${PREFIX}/lib/*.so ${TARGET_DIR}/lib/ 
#  cp ${PREFIX}/lib/*.a ${TARGET_DIR}/lib/ 
  cp ${PREFIX}/share/sipxmedialib/*.so ${TARGET_DIR}/lib/
#  rm ${TARGET_DIR}/lib/libsipXunit.a
  
  mkdir ${TARGET_DIR}/include
  cp ${PREFIX}/include/tapi/sipXtapi.h ${TARGET_DIR}/include/
  cp ${PREFIX}/include/tapi/sipXtapiEvents.h ${TARGET_DIR}/include/
  tar -czf ${TARGET_NAME}.tgz ${TARGET_DIR}
  popd > /dev/null

  echo Done
  echo ""
}


pushd ${BUILD_ROOT} > /dev/null

updateVersion ${SVN_VERSION}

build sipXportLib ${PREFIX} --disable-sip-tls
build sipXsdpLib ${PREFIX}
build sipXtackLib ${PREFIX} --disable-sipviewer --disable-sip-tls
build sipXmediaPluginMgr ${PREFIX} --disable-sip-tls
build sipXcallLib ${PREFIX} --disable-sip-tls
if [ $MP_SIPFOUNDRY == 1 ] 
then
  build sipXmediaLib ${PREFIX} --enable-local-audio
else
  build sipXmediaVoiceEngine ${PREFIX} --enable-local-audio --with-gips-vemm
fi

# package_source
# package_libs

revertVersion

popd > /dev/null
