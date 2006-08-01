include ../meta/libsrc.mak

CGICC_LIBSRC=$(LIBSRC)/cgicc

CGICC_VER = 3.2.3
CGICC_REL = 2

CGICC_SRC = $(CGICC_LIBSRC)/cgicc-$(CGICC_VER).tar.gz
CGICC_RPM = cgicc-$(CGICC_VER)-$(CGICC_REL).$(RPM_TARGET_ARCH).rpm
CGICC_DEVEL_RPM = cgicc-devel-$(CGICC_VER)-$(CGICC_REL).$(RPM_TARGET_ARCH).rpm
CGICC_SRC_RPM = cgicc-$(CGICC_VER)-$(CGICC_REL).src.rpm

all : $(CGICC_RPM) $(CGICC_DEVEL_RPM)

.PHONY: build-rpms

$(CGICC_SRC) :
	../meta/download-file $@ $(CGICC_URL)/`basename $@`

build-rpms: $(CGICC_SRC) cgicc.spec
	cp cgicc-$(CGICC_VER).tar.gz \
		$(RPMBUILD_TOPDIR)/SOURCES
	rpmbuild -ba --target $(RPM_TARGET_ARCH)-none-linux cgicc.spec

$(CGICC_RPM): build-rpms
	mv $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/$(CGICC_RPM) .

$(CGICC_DEVEL_RPM): build-rpms
	mv $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/$(CGICC_DEVEL_RPM) .

