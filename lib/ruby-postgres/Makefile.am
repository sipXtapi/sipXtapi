PGSQL_LIBSRC = $(LIBSRC)/ruby-postgres

VER = 0.7.1
REL = 3

RPM = $(DEST_RPM)/ruby-postgres-$(VER)-$(REL).$(RPM_TARGET_ARCH).rpm
SRPM = $(PGSQL_LIBSRC)/ruby-postgres-$(VER)-$(REL).src.rpm

# Where rpmbuild will do its work.
all : $(RPM)

$(SRPM) :
	$(srcdir)/config/download-file $@ $(RUBY_AUX_RPMS_URL)/SRPMS/`basename $@`

$(RPM) : $(SRPM)
	rpmbuild --rebuild $(SRPM)
	cp $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/`basename $@` $@