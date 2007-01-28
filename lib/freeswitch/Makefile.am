FS_VER = 1.0
FS_REL = snap4sipx
FS_URL=http://files.freeswitch.org
#FS_URL=http://10.2.0.4/

SRC = $(LIBSRC)/freeswitch-snapshot.tar.gz
RPMS = $(DEST_RPM)/freeswitch-$(FS_VER)-$(FS_REL).@RPM_TARGET_ARCH@.rpm 

all : $(RPMS)

$(SRC) :
	$(DOWNLOAD_FILE) $@ $(FS_URL)/`basename $@`

.PHONY: build-rpms
build-rpms: $(SRC) $(srcdir)/freeswitch_conf.patch $(srcdir)/freeswitch.spec 
	cp $(SRC) @RPMBUILD_TOPDIR@/SOURCES
	cp $(srcdir)/freeswitch_conf.patch @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux $(srcdir)/freeswitch.spec

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
