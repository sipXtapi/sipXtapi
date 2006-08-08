W3C_LIBSRC = $(LIBSRC)/w3c-libwww

VER = 5.4.0
REL = 10

SRPM = $(W3C_LIBSRC)/w3c-libwww-$(VER)-$(REL).src.rpm
RPMS = \
	$(DEST_RPM)/sipx-w3c-libwww-$(VER)-$(REL).$(RPM_TARGET_ARCH).rpm \
	$(DEST_RPM)/sipx-w3c-libwww-devel-$(VER)-$(REL).$(RPM_TARGET_ARCH).rpm

ORIG_RPM = $(subst sipx-w3c,w3c,`basename $@`)

all : $(RPMS)

$(SRPM) :
	$(srcdir)/config/download-file $@ $(W3C_URL)/SRPMS/`basename $@`

.PHONY: build-rpms
build-rpms : $(SRPM) $(srcdir)/sipx-w3c-libwww.patch
	rpm -i $(SRPM)
	patch -p0 $(RPMBUILD_TOPDIR)/SPECS/w3c-libwww.spec < $(srcdir)/sipx-w3c-libwww.patch
	rpmbuild -ba $(RPMBUILD_TOPDIR)/SPECS/w3c-libwww.spec

$(RPMS) : build-rpms
	cp $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/$(ORIG_RPM) $@

