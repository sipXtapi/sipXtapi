RPM_TARGET_ARCH = noarch
RUBY_GEMS_RPM = ruby-gems-0.8.11.noarch.rpm
RUBY_GEMS_SRCS = \
  rubygems-0.8.11.tgz \
  rails/actionmailer-1.1.5.gem \
  rails/ruby-gems-actionmailer.spec.in \
  rails/actionpack-1.11.2.gem \
  rails/ruby-gems-actionpack.spec.in \
  rails/actionwebservice-1.0.0.gem \
  rails/ruby-gems-actionwebservice.spec.in \
  rails/activerecord-1.13.2.gem \
  rails/ruby-gems-activerecord.spec.in \
  rails/activesupport-1.2.5.gem \
  rails/ruby-gems-activesupport.spec.in \
  rails/rails-1.0.0.gem \
  rails/ruby-gems-rails.spec.in \
  rails/rake-0.7.0.gem \
  rails/ruby-gems-rake.spec.in

RUBY_GEMS_SPECS = \
  rails/ruby-gems-actionmailer.spec \
  rails/ruby-gems-actionpack.spec \
  rails/ruby-gems-actionwebservice.spec \
  rails/ruby-gems-activerecord.spec \
  rails/ruby-gems-activesupport.spec \
  rails/ruby-gems-rails.spec \
  rails/ruby-gems-rake.spec

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
