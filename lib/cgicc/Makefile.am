include ../meta/libsrc.mak

CGICC_LIBSRC=$(LIBSRC)/cgicc

CGICC_VER = 3.2.3
CGICC_REL = 2

CGICC_SRC = $(CGICC_LIBSRC)/cgicc-$(CGICC_VER).tar.gz
CGICC_RPM = cgicc-$(CGICC_VER)-$(CGICC_REL).$(RPM_TARGET_ARCH).rpm
CGICC_DEVEL_RPM = cgicc-devel-$(CGICC_VER)-$(CGICC_REL).$(RPM_TARGET_ARCH).rpm
CGICC_SRC_RPM = $(RPM_REPO)/cgicc-$(CGICC_VER)-$(CGICC_REL).src.rpm

RPMS = \
	$(RPM_REPO)/$(CGICC_RPM) \
	$(RPM_REPO)/$(CGICC_DEVEL_RPM)


all : $(RPMS)

.PHONY: build-rpms

$(CGICC_SRC) :
	../meta/download-file $@ $(CGICC_URL)/`basename $@`

build-rpms: $(CGICC_SRC) $(srcdir)/cgicc.spec
	cp $(CGICC_SRC) $(RPMBUILD_TOPDIR)/SOURCES
	rpmbuild -ba --target $(RPM_TARGET_ARCH)-none-linux $(srcdir)/cgicc.spec
$(RPMS): build-rpms
	mv $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/`basename $@` $@

