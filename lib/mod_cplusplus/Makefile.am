# hint: type  "make LIBSRC=XXX" to override this location
LIBSRC=~/libsrc
MOD_CPLUSPLUS_LIBSRC=$(LIBSRC)/mod_cplusplus

MOD_CPLUSPLUS_VER = 1.5.2
MOD_CPLUSPLUS_REL = 1

# /bin/arch may return i686 but RPMs default arch is i386.
# hint: type  "make RPM_TARGET_ARCH=XXX" to override this location
RPM_TARGET_ARCH = i386
#RPM_TARGET_PLAT = rhe30

MOD_CPLUSPLUS_RPM = mod_cplusplus-$(MOD_CPLUSPLUS_VER)-$(MOD_CPLUSPLUS_REL).$(RPM_TARGET_ARCH).rpm
MOD_CPLUSPLUS_SRC = $(MOD_CPLUSPLUS_LIBSRC)/mod_cplusplus-$(MOD_CPLUSPLUS_VER).tar.gz

# Include the .spec file in any tar's we generate.
EXTRA_DIST = \
	$(srcdir)/mod_cplusplus.spec

edit = sed \
	-e 's,@APACHE2_HTTPD\@,@APACHE2_HTTPD@,g' \
	-e 's,@APACHE2_INCDIR\@,@APACHE2_INCDIR@,g' \
	-e 's,@APACHE2_HOME\@,@APACHE2_HOME@,g' \
	-e 's,@APACHE2_MOD\@,@APACHE2_MOD@,g' \
	-e 's,@APACHE2_MMN\@,@APACHE2_MMN@,g' \
	-e 's,@APACHE2_APXS\@,@APACHE2_APXS@,g' \
	-e 's,@APACHE2_VERSION\@,@APACHE2_VERSION@,g'

# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')

all : $(MOD_CPLUSPLUS_RPM)

$(MOD_CPLUSPLUS_SRC) :
	-mkdir $(XERCES_C_LIBSRC)
	wget --output-document=$@ \
		http://prdownloads.sourceforge.net/modcplusplus/`basename $@`

$(MOD_CPLUSPLUS_RPM) : $(MOD_CPLUSPLUS_SRC) mod_cplusplus.spec
	cp $(MOD_CPLUSPLUS_SRC) $(RPMBUILD_TOPDIR)/SOURCES
	rpmbuild -ba --target $(RPM_TARGET_ARCH)-none-linux mod_cplusplus.spec
	mv $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/$(MOD_CPLUSPLUS_RPM) .

# RPM Spec file
mod_cplusplus.spec : mod_cplusplus.spec.in
	$(edit) mod_cplusplus.spec.in > mod_cplusplus.spec
