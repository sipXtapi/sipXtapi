# type  "make LIBSRC=XXX" to override this location, normal make syntax
LIBSRC=~/libsrc

VERSION=1.5.0
SUN_VERSION=1_5_0
BUILD=07
REL=1

# Combine the SRC and NOSRC to create usable (non-free) rpm
JDK_LIBSRC = $(LIBSRC)/java-sun
SRC_ZIP = $(JDK_LIBSRC)/jdk-$(SUN_VERSION)_$(BUILD)-linux-i586.bin
NOSRC_RPM = $(JDK_LIBSRC)/java-$(VERSION)-sun-$(VERSION).$(BUILD)-$(REL)jpp.nosrc.rpm
RPM_SPEC=java-$(VERSION)-sun.spec
RPM=java-$(VERSION)-sun-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm

# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')

all : $(RPM)

$(SRC_ZIP) :
	@echo -e "\nMissing dependency: Download this from java.sun.com \n\t$@\n"
	@exit 1

$(NOSRC_RPM) :
	-mkdir $(JDK_LIBSRC)
	wget --output-document=$@ \
	  http://mirrors.dotsrc.org/jpackage/1.6/generic/non-free/SRPMS/`basename $@`

$(RPM) : $(NOSRC_RPM) $(SRC_ZIP) 
	rpm -i $(NOSRC_RPM)
	cp $(SRC_ZIP) $(RPMBUILD_TOPDIR)/SOURCES
	rpmbuild -ba $(RPMBUILD_TOPDIR)/SPECS/$(RPM_SPEC)
	mv $(RPMBUILD_TOPDIR)/RPMS/i586/java-$(VERSION)-sun-*$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm .
