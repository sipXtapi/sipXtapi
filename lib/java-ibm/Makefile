# type  "make LIBSRC=XXX" to override this location, normal make syntax
LIBSRC=~/libsrc

VERSION=1.5.0
IBM_VERSION=1_5_0
BUILD=2.3
REL=1

# Combine the SRC and NOSRC to create usable (non-free) rpm
JDK_LIBSRC = $(LIBSRC)/java-ibm
SRC_ZIP = $(JDK_LIBSRC)/ibm-java2-sdk-50-linux-i386.tgz
NOSRC_RPM = $(JDK_LIBSRC)/java-$(VERSION)-ibm-$(VERSION).$(BUILD)-$(REL)jpp.nosrc.rpm
RPM_SPEC=java-$(VERSION)-ibm.spec
RPM=java-$(VERSION)-ibm-$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm
UNRESTRICTED = $(JDK_LIBSRC)/unrestrict142.zip

# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')

all : $(RPM) java-ibm-unrestricted.noarch.rpm

$(UNRESTRICTED) $(SRC_ZIP) :
	@echo -e "\nMissing dependency: Download this from www.ibm.com \n\t$@\n"
	@exit 1

$(NOSRC_RPM) :
	../meta/download-file $@ $(JPKG_NONFREE_URL)/SRPMS/`basename $@`

$(RPM) : $(NOSRC_RPM) $(SRC_ZIP) 
	rpm -i $(NOSRC_RPM)
	cp $(SRC_ZIP) $(RPMBUILD_TOPDIR)/SOURCES
	rpmbuild -ba $(RPMBUILD_TOPDIR)/SPECS/$(RPM_SPEC)
	mv $(RPMBUILD_TOPDIR)/RPMS/i586/java-$(VERSION)-ibm-*$(VERSION).$(BUILD)-$(REL)jpp.i586.rpm .

java-ibm-unrestricted.noarch.rpm : $(UNRESTRICTED) java-ibm-unrestricted.spec
	cp $(UNRESTRICTED) $(RPMBUILD_TOPDIR)/SOURCES
	rpmbuild --target noarch --quiet -ba java-ibm-unrestricted.spec
	mv $(RPMBUILD_TOPDIR)/RPMS/noarch/java-ibm-unrestricted.noarch.rpm .
