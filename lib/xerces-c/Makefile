include ../meta/libsrc.mak

XERCES_C_LIBSRC=$(LIBSRC)/xerces-c

XERCES_C_VER = 2.7.0
XERCES_C_REL = 3

XERCES_C_RPM = xerces-c-$(XERCES_C_VER)-$(XERCES_C_REL).$(RPM_TARGET_ARCH).rpm
XERCES_C_VER_UNDERSCORE = $(subst .,_,$(XERCES_C_VER))
XERCES_C_TAR = $(XERCES_C_LIBSRC)/xerces-c-src_$(XERCES_C_VER_UNDERSCORE).tar.gz
XERCES_C_TAR_PATCHED = `basename $(XERCES_C_TAR)`

all : $(XERCES_C_RPM)

$(XERCES_C_TAR) :
	-mkdir $(XERCES_C_LIBSRC)
	wget --output-document=$@ \
	  $(XERCES_C_URL)/`basename $@`

# Patch the .spec file to include libxerces-depdom.so*
$(XERCES_C_TAR_PATCHED) : xerces-c-sipx.patch $(XERCES_C_TAR)
	../bin/patch-archive xerces-c-sipx.patch \
	$(XERCES_C_TAR) $(XERCES_C_TAR_PATCHED)

$(XERCES_C_RPM) : $(XERCES_C_TAR_PATCHED)
	cp $(XERCES_C_TAR_PATCHED) $(RPMBUILD_TOPDIR)/SOURCES
	rpmbuild -ta --target $(RPM_TARGET_ARCH)-none-linux $(XERCES_C_TAR_PATCHED)
	mv $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/xerces-c-*.rpm .
