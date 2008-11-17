#!/bin/sh

MAJOR_VER=3
MINOR_VER=3
REV_NUM=0
BUILD_NUM=0

LIBRARIES="sipXcallLib sipXmediaAdapterLib sipXmediaLib sipXsdpLib sipXtackLib sipXportLib"
MAILING_LIST="sipxtapi-dev@list.sipfoundry.org"

echo "Setting sipX libs version to $MAJOR_VER.$MINOR_VER.$REV_NUM.$BUILD_NUM"

# sipXtapi.h
sed -i -e "s/\(#define\s\+SIPXTAPI_VERSION\s\+\)\".*\"/\1\"$MAJOR_VER.$MINOR_VER.$REV_NUM\"/g" sipXcallLib/include/tapi/sipXtapi.h
sed -i -e "s/\(#define\s\+SIPXTAPI_BUILDNUMBER\s\+\)\".*\"/\1\"$BUILD_NUM\"/g" sipXcallLib/include/tapi/sipXtapi.h
sed -i -e "s/\(#define\s\+SIPXTAPI_BUILD_WORD\s\+\)[0-9]\+,[0-9]\+,[0-9]\+,[0-9]\+/\1$MAJOR_VER,$MINOR_VER,$REV_NUM,$BUILD_NUM/g" sipXcallLib/include/tapi/sipXtapi.h
sed -i -e "s/\(#define\s\+SIPXTAPI_FULL_VERSION\s\+\)\".*\"/\1\"$MAJOR_VER.$MINOR_VER.$REV_NUM.$BUILD_NUM\"/g" sipXcallLib/include/tapi/sipXtapi.h

# configure.ac files
for libdir in $LIBRARIES ; do
	sed -i -e "s/AC_INIT(.*)/AC_INIT($libdir, $MAJOR_VER.$MINOR_VER, $MAILING_LIST)/g" $libdir/configure.ac
done

# spec.in files
for libdir in $LIBRARIES ; do
	sed -i -e "s/Requires:\s\+\(sipx.*\lib\)\s\+>=\s\+[0-9]\+\(.[0-9]\+\)*/Requires: \1 >= $MAJOR_VER.$MINOR_VER/g" $libdir/sipx*.spec.in
done
