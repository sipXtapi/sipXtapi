#/bin/bash

PREFIX=`pwd`/build
PREFIX_ARCHIVE=`pwd`
BUILD_ROOT=`pwd`/../../..
SVN_VERSION=`svn info ${BUILD_ROOT} | grep Revision | cut -b 11-`
SVN_URL=`svn info ../../.. | grep URL | cut -b 6-`

echo "Build Root :" ${BUILD_ROOT}
echo "Build Stage:" ${PREFIX}
echo "Version    :" ${SVN_VERSION}
echo "URL        :" ${SVN_URL}

function updateVersion
{
  sed -i "s/SIPXTAPI_BUILDNUMBER.*\".*\"/SIPXTAPI_BUILDNUMBER \"${SVN_VERSION}\"/g" \
      sipXcallLib/include/tapi/sipXtapi.h
  sed -i "s/SIPXTAPI_BUILD_WORD [0-9,,]*/SIPXTAPI_BUILD_WORD 2,10,1,${SVN_VERSION}/g" \
      sipXcallLib/include/tapi/sipXtapi.h
  sed -i "s/SIPXTAPI_FULL_VERSION.*\".*\"/SIPXTAPI_FULL_VERSION \"2.10.1.${SVN_VERSION}\"/g" \
      sipXcallLib/include/tapi/sipXtapi.h
  sed -i "s/SIPXTAPI_BUILDDATE.*\".*\"/SIPXTAPI_BUILDDATE \"`date +%F`\"/g" \
      sipXcallLib/include/tapi/sipXtapi.h
}

function revertVersion
{
  svn revert sipXcallLib/include/tapi/sipXtapi.h
}

function build
{
  echo -------------------
  echo Building $1
  echo -------------------

  pushd $1 > /dev/null
  autoreconf -fi
  if [ $? -ne 0 ]
  then
    exit 1
  fi
  ./configure --prefix=$2 $3 $4 $5
  if [ $? -ne 0 ]
  then
    exit 1
  fi
  make 
  if [ $? -ne 0 ]
  then
    exit 1
  fi
  make install
  if [ $? -ne 0 ]
  then
    exit 1
  fi
  popd > /dev/null
}

function package_source
{
  echo -------------------
  echo Exporting Source
  echo -------------------
  pushd ${PREFIX_ARCHIVE} > /dev/null
  TARGET_NAME=sipXtapi_SRC_r${SVN_VERSION}_`date +%F`
  TARGET_DIR=${TARGET_NAME}
  rm -rf ${TARGET_DIR} > /dev/null
  svn export ${SVN_URL} ${TARGET_DIR}

  echo -------------------
  echo Packaging Source
  echo -------------------
  tar -czf ${TARGET_NAME}.tgz ${TARGET_DIR}
  popd > /dev/null
}

function package_libs
{
  echo -------------------
  echo Packaging Binaries
  echo -------------------

  pushd ${PREFIX_ARCHIVE} > /dev/null
  TARGET_NAME=sipXtapi_linux_r${SVN_VERSION}_`date +%F`
  TARGET_DIR=${TARGET_NAME}
  rm -rf ${TARGET_DIR} > /dev/null
  mkdir ${TARGET_DIR}
  mkdir ${TARGET_DIR}/lib
  cp ${PREFIX}/lib/*.so ${TARGET_DIR}/lib/ 
  cp ${PREFIX}/lib/*.a ${TARGET_DIR}/lib/ 
  rm ${TARGET_DIR}/lib/libsipXunit.a
  
  mkdir ${TARGET_DIR}/include
  cp ${PREFIX}/include/tapi/sipXtapi.h ${TARGET_DIR}/include/
  cp ${PREFIX}/include/tapi/sipXtapiEvents.h ${TARGET_DIR}/include/
  tar -czf ${TARGET_NAME}.tgz ${TARGET_DIR}
  popd > /dev/null

}


pushd ${BUILD_ROOT} > /dev/null

updateVersion ${SVN_VERSION}

# build sipXportLib ${PREFIX} 
# build sipXtackLib ${PREFIX} --disable-sipviewer
# build sipXmediaLib ${PREFIX} --enable-local-audio
# build sipXmediaAdapterLib ${PREFIX} 
# build sipXcallLib ${PREFIX} 

# package_source
package_libs

revertVersion

popd > /dev/null
