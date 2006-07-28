# type  "make LIBSRC=XXX" to override this location, normal make syntax
LIBSRC=~/libsrc
XERCES_C_LIBSRC=$(LIBSRC)/xerces-c

XERCES_C_VER = 2.7.0
XERCES_C_REL = 3

# /bin/arch may return i686 but RPMs default arch is i386.
RPM_TARGET_ARCH = i386
#RPM_TARGET_PLAT = rhe30

XERCES_C_RPM = xerces-c-$(XERCES_C_VER)-$(XERCES_C_REL).$(RPM_TARGET_ARCH).rpm

# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')

all : $(XERCES_C_RPM)

# Since the Xerces people name their tar's using the version number with
# underscores, we have to manually maintain the version number here.
XERCES_C_TAR = $(XERCES_C_LIBSRC)/xerces-c-src_2_7_0.tar.gz
XERCES_C_TAR_PATCHED = `basename $(XERCES_C_TAR)`

$(XERCES_C_TAR) :
	-mkdir $(XERCES_C_LIBSRC)
	wget --output-document=$@ \
	  http://www.apache.org/dist/xml/xerces-c/source/`basename $@`

# Patch the .spec file to include libxerces-depdom.so*
$(XERCES_C_TAR_PATCHED) : xerces-c-sipx.patch $(XERCES_C_TAR)
	../bin/patch-archive xerces-c-sipx.patch \
	$(XERCES_C_TAR) $(XERCES_C_TAR_PATCHED)

$(XERCES_C_RPM) : $(XERCES_C_TAR_PATCHED)
	cp $(XERCES_C_TAR_PATCHED) $(RPMBUILD_TOPDIR)/SOURCES
	rpmbuild -ta --target $(RPM_TARGET_ARCH)-none-linux $(XERCES_C_TAR_PATCHED)
	mv $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/xerces-c-*.rpm .
