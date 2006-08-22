VER = 1.5.2
REL = 1

SRC = $(LIBSRC)/mod_cplusplus/mod_cplusplus-$(VER).tar.gz

RPMS = $(DEST_RPM)/mod_cplusplus-$(VER)-$(REL).$(RPM_TARGET_ARCH).rpm

SRPM = $(DEST_SRPM)/mod_cplusplus-$(VER)-$(REL).src.rpm

edit = sed \
	-e 's,@APACHE2_HTTPD\@,@APACHE2_HTTPD@,g' \
	-e 's,@APACHE2_INCDIR\@,@APACHE2_INCDIR@,g' \
	-e 's,@APACHE2_HOME\@,@APACHE2_HOME@,g' \
	-e 's,@APACHE2_MOD\@,@APACHE2_MOD@,g' \
	-e 's,@APACHE2_MMN\@,@APACHE2_MMN@,g' \
	-e 's,@APACHE2_APXS\@,@APACHE2_APXS@,g' \
	-e 's,@APACHE2_VERSION\@,@APACHE2_VERSION@,g'

all : $(SRPM) $(RPMS)

$(SRC) :
	$(DOWNLOAD_FILE) $@ $(MOD_CPLUSPLUS_URL)/`basename $@`

.PHONY: build-rpms
build-rpms: $(SRC) mod_cplusplus.spec
	cp $(SRC) $(RPMBUILD_TOPDIR)/SOURCES
	rpmbuild -ba --target $(RPM_TARGET_ARCH)-none-linux mod_cplusplus.spec

# RPM Spec file
mod_cplusplus.spec : $(srcdir)/mod_cplusplus.spec.in
	$(edit) $(srcdir)/mod_cplusplus.spec.in > mod_cplusplus.spec

$(SRPM): build-rpms
	mv $(RPMBUILD_TOPDIR)/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/`basename $@` $@	

