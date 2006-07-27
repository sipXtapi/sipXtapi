PACKAGE_VER = 5.4.0
PACKAGE_REL = 3
PACKAGE_NAME = sipx-w3c-libwww
TAR_NAME = w3c-libwww

# /bin/arch may return i686 but RPMs default arch is i386.
RPM_TARGET_ARCH = i386
#RPM_TARGET_PLAT = rhe30

PACKAGE_RPM = $(PACKAGE_NAME)-$(PACKAGE_VER)-$(PACKAGE_REL).$(RPM_TARGET_ARCH).rpm
PACKAGE_DEVEL_RPM = $(PACKAGE_NAME)-devel-$(PACKAGE_VER)-$(PACKAGE_REL).$(RPM_TARGET_ARCH).rpm
PACKAGE_SRC_RPM = $(PACKAGE_NAME)-$(PACKAGE_VER)-$(PACKAGE_REL).src.rpm

# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')

all : $(PACKAGE_RPM) $(PACKAGE_DEVEL_RPM)

.PHONY: build-rpms

build-rpms: $(TAR_NAME)-$(PACKAGE_VER).tgz $(PACKAGE_NAME).spec
	../bin/archive-rootdir $(TAR_NAME)-$(PACKAGE_VER).tgz \
		w3c-libwww-$(PACKAGE_VER) sipx-w3c-libwww-$(PACKAGE_VER)
	cp $(TAR_NAME)-$(PACKAGE_VER).tgz.new \
		$(RPMBUILD_TOPDIR)/SOURCES/$(TAR_NAME)-$(PACKAGE_VER).tgz
	rpmbuild -ba --target $(RPM_TARGET_ARCH)-none-linux $(PACKAGE_NAME).spec

$(PACKAGE_RPM): build-rpms
	mv $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/$(PACKAGE_RPM) .

$(PACKAGE_DEVEL_RPM): build-rpms
	mv $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/$(PACKAGE_DEVEL_RPM) .

