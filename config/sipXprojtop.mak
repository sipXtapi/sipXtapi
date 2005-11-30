## Makefile components common to the top level of any sipX project

## BUILDSTAMP records the subversion revision, date/time, and host of the build
##   it also creates the cpp file config/sipX-buildstamp.cpp with the same information.

## The SVN-VERSION file is created to record the version 
##  in the distribution tarball; it is not used in a subversion working copy
##  (see the BUILDSTAMP rule below).
.PHONY: SVN-VERSION
SVN-VERSION: 
	@if test -d @top_srcdir@/.svn ; \
	then \
		echo "Generating SVN-VERSION from svnversion"; \
		svnversion @top_srcdir@ \
		| perl -p \
			-e 'm /(\d+)/ && do { $$padded=sprintf( "%06d", $$1 ); s/\d+/$$padded/; };' \
			-e 's/:/./; s/M/.M/;' \
		> @abs_top_builddir@/SVN-VERSION ; \
	elif test -r @abs_top_srcdir@/SVN-EXPORT-VERSION ; \
	then \
		echo "Copying SVN-VERSION from SVN-EXPORT-VERSION"; \
		cp @abs_top_srcdir@/SVN-EXPORT-VERSION @abs_top_builddir@/SVN-VERSION ; \
	elif test -r @abs_top_srcdir@/../SVN-EXPORT-VERSION ; \
	then \
		echo "Copying SVN-VERSION from top level SVN-EXPORT-VERSION"; \
		cp @abs_top_srcdir@/../SVN-EXPORT-VERSION @abs_top_builddir@/SVN-VERSION ; \
	else \
		echo "Unknown SVN-VERSION"; \
		echo '0.unknown' > @abs_top_builddir@/SVN-VERSION ; \
	fi
	@echo -n "SVN-VERSION=" ; cat @abs_top_builddir@/SVN-VERSION; echo ""

SVN-EXPORT-VERSION: SVN-VERSION
	cp SVN-VERSION SVN-EXPORT-VERSION

all: BUILDSTAMP

.PHONY: BUILDSTAMP
BUILDSTAMP: SVN-VERSION
	@echo "Generating BUILDSTAMP" 
	sipxSvnVersion=`cat @abs_top_builddir@/SVN-VERSION` ; \
	sipxBuildTime=`date -u --iso-8601=seconds | sed 's/+0000//'` ; \
	sipxBuildHost=`hostname --short` ; \
	SIPX_BUILDSTAMP="$${sipxSvnVersion} $${sipxBuildTime} $${sipxBuildHost}" ; \
	echo "$${SIPX_BUILDSTAMP}" > BUILDSTAMP ; \
	${LocalizeSipXconfig} < $(srcdir)/config/sipX-buildstamp.cpp.in \
		      -e "s/@SIPX_BUILDSTAMP\@/$${SIPX_BUILDSTAMP}/" \
		      -e "s/sipx/SipX/" \
		> config/@PACKAGE@-buildstamp.cpp ; \
	${LocalizeSipXconfig} < $(srcdir)/config/sipX-buildstamp.h.in \
		      -e "s/@SIPX_BUILDSTAMP\@/$${SIPX_BUILDSTAMP}/" \
		      -e "s/sipx/SipX/" \
		> config/@PACKAGE@-buildstamp.h

.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')

rpm : dist
	rpmbuild -ta --define="buildno $(shell cat @abs_top_builddir@/SVN-VERSION)" @PACKAGE@-$(VERSION).tar.gz
	mv -f $(RPMBUILD_TOPDIR)/SRPMS/@PACKAGE@-$(VERSION)-*.rpm .
	mv -f $(RPMBUILD_TOPDIR)/RPMS/*/@PACKAGE@*-$(VERSION)-*.rpm .
	md5sum @PACKAGE@-$(VERSION).tar.gz >@PACKAGE@-$(VERSION).tar.gz.md5

# RPM Spec file
@PACKAGE@.spec : @PACKAGE@.spec.in
	$(LocalizeSipXconfig) < $(srcdir)/@PACKAGE@.spec.in > @PACKAGE@.spec

dist-hook : $(distdir)/@PACKAGE@.spec

# 'rpmbuild -ta' searches root of tarball for first *.spec file to build 
# RPM from
$(distdir)/@PACKAGE@.spec:
	cp @PACKAGE@.spec $(distdir)

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

# CONFIG_DISTCLEANFILES are files to be added to DISTCLEANFILES in any
# makefile that includes config as an external subdirectory.
CONFIG_DISTCLEANFILES = \
	config/config.sub \
	config/depcomp \
	config/install-sh \
	config/ltmain.sh \
	config/missing \
	config/@PACKAGE@-buildstamp.cpp \
	config/@PACKAGE@-buildstamp.h
