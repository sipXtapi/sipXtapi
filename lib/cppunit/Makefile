CPPUNIT_VER = 1.10.2
CPPUNIT_REL = 1

# /bin/arch may return i686 but RPMs default arch is i386.
RPM_TARGET_ARCH = i386
#RPM_TARGET_PLAT = rhe30

CPPUNIT_SIPX_RPM = cppunit-$(CPPUNIT_VER)-$(CPPUNIT_REL).$(RPM_TARGET_ARCH).rpm
CPPUNIT_TAR = cppunit-$(CPPUNIT_VER).tar.gz

# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')

all: $(CPPUNIT_SIPX_RPM)

# cppunit has a rpmbuild error that some files aren't part of
# spec but are part of 'make install'. Fix that here.
$(CPPUNIT_TAR) : cppunit-sipx.patch orig/$(CPPUNIT_TAR) 
	../bin/patch-archive cppunit-sipx.patch orig/$(CPPUNIT_TAR) $(CPPUNIT_TAR)

$(CPPUNIT_SIPX_RPM) : $(CPPUNIT_TAR)
	rpmbuild -ta --target $(RPM_TARGET_ARCH)-none-linux cppunit-$(CPPUNIT_VER).tar.gz
	mv $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/cppunit-$(CPPUNIT_VER)-$(CPPUNIT_REL).*.rpm $(CPPUNIT_SIPX_RPM)
