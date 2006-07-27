CGICC_VER = 3.2.3
CGICC_REL = 2

# /bin/arch may return i686 but RPMs default arch is i386.
RPM_TARGET_ARCH = i386
#RPM_TARGET_PLAT = rhe30

CGICC_RPM = cgicc-$(CGICC_VER)-$(CGICC_REL).$(RPM_TARGET_ARCH).rpm
CGICC_DEVEL_RPM = cgicc-devel-$(CGICC_VER)-$(CGICC_REL).$(RPM_TARGET_ARCH).rpm
CGICC_SRC_RPM = cgicc-$(CGICC_VER)-$(CGICC_REL).src.rpm

# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')

all : $(CGICC_RPM) $(CGICC_DEVEL_RPM)

.PHONY: build-rpms

build-rpms: cgicc-$(CGICC_VER).tar.gz cgicc.spec
	cp cgicc-$(CGICC_VER).tar.gz \
		$(RPMBUILD_TOPDIR)/SOURCES
	rpmbuild -ba --target $(RPM_TARGET_ARCH)-none-linux cgicc.spec

$(CGICC_RPM): build-rpms
	mv $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/$(CGICC_RPM) .

$(CGICC_DEVEL_RPM): build-rpms
	mv $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/$(CGICC_DEVEL_RPM) .

