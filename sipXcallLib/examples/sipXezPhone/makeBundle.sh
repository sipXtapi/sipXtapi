#!/bin/bash

# Build the OS X sipXezPhone bundle and disk image.
# 
# Copyright (C) 2005-2006 SIPez LLC.
# Licensed to SIPfoundry under a Contributor Agreement.
# 
# Copyright (C) 2004-2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# You must have already compiled all required sipX libraries and sipXezPhone.
# This script just collects everything together into an OS X bundle, then
# creates a compressed disk image for distribution.

# sipX paths can be given through environment variables
if [ -z "$SIPX_SRC_PATH" -o -z "$SIPX_INST_PATH" ]
then
	if [ $# -lt 2 -o $# -gt 3 ]
	then
		echo "Usage: $0 <sipX source path> <sipX installed path> [bundle name]"
		exit
	fi
	SIPX_SRC_PATH="$1"
	SIPX_INST_PATH="$2"
	shift 2
fi

APP_NAME=${1:-sipXezPhone}

# Derived paths
APP_PATH=$APP_NAME.app
SIPX_BIN_PATH=$SIPX_INST_PATH/bin
SIPX_LIB_PATH=$SIPX_INST_PATH/lib
SIPX_RES_PATH=$SIPX_SRC_PATH/sipXcallLib/examples/sipXezPhone/res

# Display an error message and quit
function die() {
	echo "$@" >&2
	exit 1
}

# Error checks
[ -f $SIPX_BIN_PATH/sipXezPhone ] || die "No sipXezPhone binary"
[ -d $SIPX_RES_PATH ] || die "No sipXezPhone resources"
[ -d $APP_PATH ] && die "Bundle already exists"

# Bundle directories
APP_CONT_PATH=$APP_PATH/Contents
APP_MAC_PATH=$APP_CONT_PATH/MacOS
APP_MAIN_PATH=$APP_CONT_PATH/Resources
APP_RES_PATH=$APP_MAIN_PATH/res
APP_LIB_PATH=$APP_MAIN_PATH/lib

# Create the bundle directory structure
mkdir $APP_PATH
mkdir $APP_CONT_PATH
mkdir $APP_MAC_PATH
mkdir $APP_MAIN_PATH
mkdir $APP_RES_PATH
mkdir $APP_LIB_PATH

# Write the main executable script
cat > $APP_MAC_PATH/sipXezPhone << EOF
#!/bin/sh
cd "\`dirname "\$0"\`"/../Resources
DYLD_LIBRARY_PATH=lib
export DYLD_LIBRARY_PATH
rm -f ezphone.log
exec ./sipXezPhone "\$@"
EOF
chmod +x $APP_MAC_PATH/sipXezPhone

# Write the Info.plist file
cat > $APP_CONT_PATH/Info.plist << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleExecutable</key>
	<string>sipXezPhone</string>
	<key>CFBundleIconFile</key>
	<string>sipXezPhone.icns</string>
	<key>CFBundleIdentifier</key>
	<string>org.sipfoundry.sipXezPhone</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>sipXezPhone</string>
	<key>LSMinimumSystemVersion</key>
	<string>10.3</string>
</dict>
</plist>
EOF

# Write the PkgInfo file
echo -n "AAPL????" > $APP_CONT_PATH/PkgInfo

# Write the version.plist file
cat > $APP_CONT_PATH/version.plist << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>BuildVersion</key>
	<string>1</string>
	<key>CFBundleShortVersionString</key>
	<string>1.0</string>
	<key>CFBundleVersion</key>
	<string>1.0</string>
	<key>ProjectName</key>
	<string>sipXezPhone</string>
	<key>SourceVersion</key>
	<string>1</string>
</dict>
</plist>
EOF

# Write the CustomInfo.plist file
cat > $APP_MAIN_PATH/CustomInfo.plist << EOF
{
    CFBundleIconFile = "sipXezPhone.icns";
}
EOF

# Copy res directory from source control
cp -r $SIPX_RES_PATH/* $APP_RES_PATH
rm -f $APP_RES_PATH/sipXezPhone.ico
mv $APP_RES_PATH/sipXezPhone.icns $APP_MAIN_PATH

# Copy the binary itself
cp $SIPX_BIN_PATH/sipXezPhone $APP_MAIN_PATH

# List libraries used by a binary
function ldd() {
	otool -L "$@" | grep -v "^[^ 	]" | sed -e 's/[ 	]*\(.*\) ([^(]*$/\1/' | grep -v ^/usr/ | grep -v ^/System/ | sort | uniq
}

# Copy libraries into the bundle
function copy_in() {
	nothing=1
	while read LIB
	do
		[ -f $APP_MAIN_PATH/lib/"`basename "$LIB"`" ] && continue
		[ -f "$LIB" ] || LIB=$SIPX_LIB_PATH/$LIB
		cp "$LIB" $APP_MAIN_PATH/lib && nothing=0 || echo "Failed to copy `basename "$LIB"`"
	done
	return $nothing
}

# Copy an initial set of libraries
ldd $APP_MAIN_PATH/sipXezPhone | copy_in
# Keep copying libraries until we have all dependencies
while :;
do
	ldd $APP_LIB_PATH/* | copy_in || break
done

# Make sure all libraries are writable and strip them
chmod +w $APP_LIB_PATH/*
strip -S $APP_MAIN_PATH/sipXezPhone $APP_LIB_PATH/*

# Calculate the size of the bundle and add 10%
SIZE="`du -sh sipXezPhone.app | awk '{print $1}' | tr -d M`"
SIZE="`echo "$SIZE*11/10" | bc -q`m"

# Make a disk image and mount it
hdiutil create -size $SIZE -fs HFS+ -volname $APP_NAME /tmp/$APP_NAME.dmg > /dev/null
hdiutil attach /tmp/$APP_NAME.dmg > /dev/null

# Copy the bundle into it
cp -r $APP_PATH /Volumes/$APP_NAME

# Unmount it and compress the image
hdiutil detach "`mount | grep /Volumes/$APP_NAME | awk '{print $1}' | cut -d/ -f 3`" > /dev/null
hdiutil convert -format UDZO -o /tmp/$APP_NAME.2.dmg /tmp/$APP_NAME.dmg > /dev/null

# Move the disk image next to the bundle
rm /tmp/$APP_NAME.dmg
mv /tmp/$APP_NAME.2.dmg $APP_NAME.dmg
