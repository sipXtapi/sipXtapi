FSCONF_VER = 1.0
FSCONF_REL = 2

SRC = sipxconfig-freeswitch.tar.gz
RPMS = $(DEST_RPM)/sipxconfig-freeswitch-$(FSCONF_VER)-$(FSCONF_REL).@RPM_TARGET_ARCH@.rpm 

all : $(RPMS)

$(SRC) :
	tar cvzf $(SRC) `cat files.list | sed 's/^\//sipxconfig-freeswitch\//'`
	cp files.list @RPMBUILD_TOPDIR@/SOURCES/sipxconfig-freeswitch-files.list

.PHONY: build-rpms
build-rpms: $(SRC) sipXfreeswitch.spec
	cp $(SRC) @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux sipXfreeswitch.spec

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
