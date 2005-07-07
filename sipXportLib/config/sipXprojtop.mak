## Makefile components common to the top level of any sipX project

## BUILDSTAMP records the subversion revision, date/time, and host of the build
##   it also creates the cpp file config/sipX-buildstamp.cpp with the same information.
all: BUILDSTAMP

## The SVN-VERSION file is created to record the version 
##  in the distribution tarball; it is not used in a subversion working copy
##  (see the BUILDSTAMP rule below).
SVN-VERSION:
	svnversion @top_srcdir@ > SVN-VERSION

.PHONY: BUILDSTAMP
BUILDSTAMP:
	  if test -d @top_srcdir@/.svn \
	  ;then \
	    sipxSvnVersion=`svnversion @top_srcdir@` \
	  ;elif test -f @top_srcdir@/SVN-VERSION \
	  ;then \
	    sipxSvnVersion=`cat @top_srcdir@/SVN-VERSION` \
	  ;fi \
	  ;if test -z "$${sipxSvnVersion}" -o "export" = "$${sipxSvnVersion}" \
	  ;then \
	    sipxSvnVersion="SVN-UNKNOWN" \
	  ;fi \
	  ;sipxBuildTime=`date -u --iso-8601=seconds | sed 's/+0000//'` \
	  ;sipxBuildHost=`hostname --short` \
	  ;SIPX_BUILDSTAMP="r$${sipxSvnVersion} $${sipxBuildTime} $${sipxBuildHost}" \
	  ;echo "$${SIPX_BUILDSTAMP}" > BUILDSTAMP \
	  ;${LocalizeSipXconfig} \
		-e "s/@SIPX_BUILDSTAMP\@/$${SIPX_BUILDSTAMP}/" \
		-e "s/sipx/SipX/" \
	    $(srcdir)/config/sipX-buildstamp.cpp.in \
	  > config/@PACKAGE@-buildstamp.cpp \
	  ;${LocalizeSipXconfig} \
		-e "s/@SIPX_BUILDSTAMP\@/$${SIPX_BUILDSTAMP}/" \
		-e "s/sipx/SipX/" \
	    $(srcdir)/config/sipX-buildstamp.h.in \
	  > config/@PACKAGE@-buildstamp.h

# automake doesn't like this inside the conditional
.PHONY : doc
# 
# If Doxygen is enabled
#
if DOC
doc_SUBDIRS = doc

doc :
	cd doc && $(MAKE) $(AM_MAKEFLAGS) all

endif

