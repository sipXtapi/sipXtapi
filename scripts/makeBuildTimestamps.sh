#!/bin/bash

SIPX_REV="3.3"
SVN_REV=`svn info | grep Revision | awk '{print $2}'`

LIB_NAMES="sipXportLib sipXsdpLib sipXtackLib sipXmediaLib sipXmediaAdapterLib sipXcallLib"

for LIB_NAME in ${LIB_NAMES}
do
  if [ ! -d $LIB_NAME ]
  then
    echo Must be in sipX root directory containing: $LIB_NAME
    exit 1
  fi
  LOWER_LIB_NAME=`echo ${LIB_NAME} | tr "[:upper:]" "[:lower:]"`
  #echo svn: ${SVN_REV} sipX rev: ${SIPX_REV}
  #echo Name: ${LIB_NAME} \(${LOWER_LIB_NAME}\)
  if [ ! -f ${LIB_NAME}/config/${LOWER_LIB_NAME}-buildstamp.h ]
  then
    echo creating: ${LIB_NAME}/config/${LOWER_LIB_NAME}-buildstamp.h:
    sed -e "s/@PACKAGE@/${LIB_NAME}/" ${LIB_NAME}/config/sipX-buildstamp.h.in | sed -e 's/sipX/SipX/' -e 's/Lib/lib/' > ${LIB_NAME}/config/${LOWER_LIB_NAME}-buildstamp.h
  fi

  echo creating: ${LIB_NAME}/config/${LOWER_LIB_NAME}-buildstamp.cpp
  sed -e "s/@PACKAGE@/${LIB_NAME}/" ${LIB_NAME}/config/sipX-buildstamp.cpp.in | \
    sed -e "s/@SIPX_BUILDSTAMP@/${SVN_REV}/" | \
    sed -e "s/@VERSION@/${SIPX_REV}/" | \
    sed -e 's/sipX/SipX/' -e 's/Lib/lib/' > ${LIB_NAME}/config/${LOWER_LIB_NAME}-buildstamp.cpp


done
