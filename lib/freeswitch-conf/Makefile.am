FSCONF_VER = 1.0
FSCONF_REL = 1

SRC = sipxconfig-freeswitch.tar.gz
RPMS = $(DEST_RPM)/sipxconfig-freeswitch-$(FSCONF_VER)-$(FSCONF_REL).@RPM_TARGET_ARCH@.rpm 

all : $(RPMS)

$(SRC) :
	mkdir `basename $(SRC) .tar.gz`
	cp -rp files/* `basename $(SRC) .tar.gz`
	tar cvzf $(SRC) `basename $(SRC) .tar.gz`
	rm -rf `basename $(SRC) .tar.gz`

.PHONY: build-rpms
build-rpms: $(SRC) sipXfreeswitch.spec
	cp $(SRC) @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux sipXfreeswitch.spec

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
