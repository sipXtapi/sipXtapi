SUDO = sudo

all: BUILDSTAMP

## BUILDSTAMP records the subversion revision, date/time, and host of the build
##   it also creates the cpp file config/sipX-buildstamp.cpp with the same information.
.PHONY: BUILDSTAMP
BUILDSTAMP:
	@echo "Generating BUILDSTAMP" 
	sipxSvnVersion=`@SVN_VERSION@` ; \
	sipxBuildTime=`date -u +%Y-%m-%dT%H:%M:%S` ; \
	sipxBuildHost=`uname -n` ; \
	SIPX_BUILDSTAMP="$${sipxSvnVersion} $${sipxBuildTime} $${sipxBuildHost}" ; \
	echo "$${SIPX_BUILDSTAMP}" > BUILDSTAMP ; \
	${LocalizeSipXconfig} \
		-e "s/@SIPX_BUILDSTAMP\@/$${SIPX_BUILDSTAMP}/" \
		-e "s/sipx/SipX/" \
	    $(srcdir)/config/sipX-buildstamp.cpp.in \
	> config/@PACKAGE@-buildstamp.cpp ; \
	${LocalizeSipXconfig} \
		-e "s/@SIPX_BUILDSTAMP\@/$${SIPX_BUILDSTAMP}/" \
		-e "s/sipx/SipX/" \
	    $(srcdir)/config/sipX-buildstamp.h.in \
	> config/@PACKAGE@-buildstamp.h

SVN_VERSION:
	echo "@SVN_VERSION@" > SVN_VERSION

RPM=$(DEST_RPM)/@PACKAGE@-$(VERSION)-@SVN_VERSION@.$(RPM_TARGET_ARCH).rpm 
DEVEL_RPM=$(DEST_RPM)/@PACKAGE@-devel-$(VERSION)-@SVN_VERSION@.$(RPM_TARGET_ARCH).rpm
DEBUG_RPM=$(DEST_RPM)/@PACKAGE@-debug-$(VERSION)-@SVN_VERSION@.$(RPM_TARGET_ARCH).rpm

RPMS = $(RPM) $(DEVEL_RPM)
SRPM = $(DEST_SRPM)/@PACKAGE@-$(VERSION)-@SVN_VERSION@.src.rpm

RPM_INSTALL_FLAGS = -F --nodeps --quiet --noscripts --notriggers

.PHONY: rpm
rpm : $(RPMS)

.PHONY: install-rpms
install-rpms : $(RPMS)
	$(SUDO) rpm $(RPM_INSTALL_FLAGS) $(RPMS)

list-rpms :
	@echo $(RPMS)

.PHONY : build-rpms
build-rpms : dist additional-package-files
	rpmbuild -ta --define="buildno @SVN_VERSION@" @PACKAGE@-$(VERSION).tar.gz

$(RPMS) : $(SRPM)
	cp $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/`basename $@` $@

$(SRPM) : build-rpms
	cp $(RPMBUILD_TOPDIR)/SRPMS/`basename $@` $@

.PHONY : additional-package-files
additional-package-files: \
	@PACKAGE@-$(VERSION).tar.gz.md5 \
	@PACKAGE@-$(VERSION).tar.bz2 \
	@PACKAGE@-$(VERSION).tar.bz2.md5

@PACKAGE@-$(VERSION).tar.gz: dist

@PACKAGE@-$(VERSION).tar.gz.md5: @PACKAGE@-$(VERSION).tar.gz
	md5sum @PACKAGE@-$(VERSION).tar.gz > @PACKAGE@-$(VERSION).tar.gz.md5

@PACKAGE@-$(VERSION).tar.bz2: @PACKAGE@-$(VERSION).tar.gz
	zcat @PACKAGE@-$(VERSION).tar.gz \
	| bzip2 --compress --stdout > @PACKAGE@-$(VERSION).tar.bz2

@PACKAGE@-$(VERSION).tar.bz2.md5: @PACKAGE@-$(VERSION).tar.bz2
	md5sum @PACKAGE@-$(VERSION).tar.bz2 > @PACKAGE@-$(VERSION).tar.bz2.md5

# RPM Spec file
@PACKAGE@.spec : @PACKAGE@.spec.in
	$(LocalizeSipXconfig) $(srcdir)/@PACKAGE@.spec.in > @PACKAGE@.spec

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
