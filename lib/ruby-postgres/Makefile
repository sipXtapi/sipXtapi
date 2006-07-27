RPM_TARGET_ARCH = noarch
RUBY_GEMS_RPM = ruby-gems-0.8.11.noarch.rpm
RUBY_GEMS_SRCS = \
  postgres/ruby-postgres-0.7.1.tar.gz

RUBY_GEMS_SPECS = \
  postgres/ruby-postgres.spec

# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')

all : $(RUBY_GEMS_RPM)

$(RUBY_GEMS_RPM) : $(RUBY_GEMS_SRCS)
	list='$(RUBY_GEMS_SRCS)'; for sfile in $$list; do \
    cp $$sfile $(RPMBUILD_TOPDIR)/SOURCES; \
    done
	rpmbuild -bb --target $(RPM_TARGET_ARCH)-none-linux ruby-gems.spec
	mv $(RPMBUILD_TOPDIR)/RPMS/noarch/ruby-gems-0.8.11-2.noarch.rpm .
#	sudo rpm --upgrade --force ruby-gems-0.8.11-2.noarch.rpm
#	list='$(RUBY_GEMS_SPECS)'; for sfile in $$list; do \
#    rpmbuild -bb --target $(RPM_TARGET_ARCH)-none-linux $$sfile; \
#    mv $(RPMBUILD_TOPDIR)/RPMS/noarch/ruby-*.rpm .; done
