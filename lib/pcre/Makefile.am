include ../meta/libsrc.mak

RPM_TARGET_ARCH = i386
PCRE_RPM = pcre-4.5-2.i386.rpm
PCRE_DEVEL_RPM = pcre-devel-4.5-2.i386.rpm
PCRE_SRC = pcre-4.5.tar.bz2

all : $(PCRE_RPM)

$(PCRE_RPM) : $(PCRE_SRC) pcre-3.9-libtool.patch
	cp $(PCRE_SRC) $(RPMBUILD_TOPDIR)/SOURCES
	cp pcre-3.9-libtool.patch $(RPMBUILD_TOPDIR)/SOURCES
	rpmbuild -bb --target $(RPM_TARGET_ARCH)-none-linux pcre.spec
	mv $(RPMBUILD_TOPDIR)/RPMS/i386/$(PCRE_RPM) .
	mv $(RPMBUILD_TOPDIR)/RPMS/i386/$(PCRE_DEVEL_RPM) .
