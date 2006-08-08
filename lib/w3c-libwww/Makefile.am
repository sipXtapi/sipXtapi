include ../meta/libsrc.mak

W3C_LIBSRC = $(LIBSRC)/w3c-libwww

W3C_VER = 5.4.0
W3C_REL = 10

TARGET_DIR = $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)
W3C_RPM = w3c-libwww-$(W3C_VER)-$(W3C_REL).$(RPM_TARGET_ARCH).rpm
W3C_DEVEL_RPM = w3c-libwww-devel-$(W3C_VER)-$(W3C_REL).$(RPM_TARGET_ARCH).rpm
W3C_SRPM = $(W3C_LIBSRC)/w3c-libwww-$(W3C_VER)-$(W3C_REL).src.rpm

SIPX_W3C_RPM = $(subst w3c,sipx-w3c,$(W3C_RPM))
SIPX_W3C_DEVEL_RPM = $(subst w3c,sipx-w3c,$(W3C_DEVEL_RPM))

all : $(SIPX_W3C_RPM) $(SIPX_W3C_DEVEL_RPM)

$(W3C_SRPM) :
	../meta/download-file $@ $(RE4_W3C_URL)/SRPMS/`basename $@`

$(TARGET_DIR)/$(W3C_RPM) $(TARGET_DIR)/$(W3C_DEVEL_RPM) : $(W3C_SRPM)
	rpm -i $(W3C_SRPM)
	patch -p0 $(RPMBUILD_TOPDIR)/SPECS/w3c-libwww.spec < sipx-w3c-libwww.patch
	rpmbuild -ba $(RPMBUILD_TOPDIR)/SPECS/w3c-libwww.spec

$(SIPX_W3C_RPM) : $(TARGET_DIR)/$(W3C_RPM)
	cp $(TARGET_DIR)/$(W3C_RPM) $(SIPX_W3C_RPM)

$(SIPX_W3C_DEVEL_RPM) : $(TARGET_DIR)/$(W3C_DEVEL_RPM)
	cp $(TARGET_DIR)/$(W3C_DEVEL_RPM) $(SIPX_W3C_DEVEL_RPM)
