# simply download src rpm from dlutter and rebuild it :)

include ../meta/libsrc.mak

PGSQL_LIBSRC := $(LIBSRC)/ruby-postgres

PGSQL_VER := 0.7.1
PGSQL_REL := 3

PGSQL_RPM = ruby-postgres-$(PGSQL_VER)-$(PGSQL_REL).i386.rpm
PGSQL_SRPM = $(PGSQL_LIBSRC)/ruby-postgres-$(PGSQL_VER)-$(PGSQL_REL).src.rpm

# Where rpmbuild will do its work.
all : $(PGSQL_RPM)

$(PGSQL_SRPM) :
	../meta/download-file $@ $(RUBY_AUX_RPMS_URL)/SRPMS/`basename $@`

$(PGSQL_RPM) : $(PGSQL_SRPM)
	rpmbuild --rebuild $(PGSQL_SRPM)
	cp $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/$(PGSQL_RPM) .