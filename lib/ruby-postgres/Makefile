# simply download src rpm from dlutter and rebuild it :)

# type  "make LIBSRC=XXX" to override this location, normal make syntax
LIBSRC=~/libsrc
PGSQL_LIBSRC := $(LIBSRC)/ruby-postgres

PGSQL_VER := 0.7.1
PGSQL_REL := 3

PGSQL_RPM = ruby-postgres-$(PGSQL_VER)-$(PGSQL_REL).i386.rpm
PGSQL_SRPM = $(PGSQL_LIBSRC)/ruby-postgres-$(PGSQL_VER)-$(PGSQL_REL).src.rpm

# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')

all : $(PGSQL_RPM)

$(PGSQL_SRPM) :
	-mkdir $(PGSQL_LIBSRC)
	wget --output-document=$@ \
	  http://people.redhat.com/dlutter/yum/SRPMS/`basename $@`

$(PGSQL_RPM) : $(PGSQL_SRPM)
	rpmbuild --rebuild $(PGSQL_SRPM)
	cp $(RPMBUILD_TOPDIR)/RPMS/i386/$(PGSQL_RPM) .