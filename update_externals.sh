#!/bin/sh

OLD_PATH="main/"
NEW_PATH="branches/sipXtapi-3.2/"

EXT_DIRS="sipXportLib sipXportLib/include sipXsdpLib sipXtackLib sipXtackLib/include sipXmediaLib sipXmediaAdapterLib sipXcallLib "

for ext_dir in $EXT_DIRS ; do
	EXT_VAL=`svn propget svn:externals $ext_dir | sed -e "s|https://scm.sipfoundry.org/rep/sipX/$OLD_PATH|https://scm.sipfoundry.org/rep/sipX/$NEW_PATH|g"`
	echo
	echo "Setting svn externals in $ext_dir to:"
	echo $EXT_VAL
	svn propset svn:externals "$EXT_VAL" $ext_dir >/dev/null
done
