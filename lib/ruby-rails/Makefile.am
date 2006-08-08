RAILS_LIBSRC=$(LIBSRC)/ruby-rails

RAILS = \
	ruby-gems-actionmailer-1.1.5-1.noarch.rpm \
	ruby-gems-actionpack-1.11.2-1.noarch.rpm \
	ruby-gems-actionwebservice-1.0.0-1.noarch.rpm \
	ruby-gems-activerecord-1.13.2-1.noarch.rpm \
	ruby-gems-activesupport-1.2.5-1.noarch.rpm \
	ruby-gems-rails-1.0.0-1.noarch.rpm \
	ruby-gems-rake-0.7.0-1.noarch.rpm

DOWNLOAD = $(foreach file,$(RAILS),$(RAILS_LIBSRC)/$(file))
RPMS = $(foreach file,$(RAILS),$(DEST_RPM)/$(file))

all : $(RPMS)

$(DOWNLOAD) :
	$(srcdir)/config/download-file $@ $(RUBY_AUX_RPMS_URL)/RPMS/`basename $@`

$(RPMS) : $(DOWNLOAD)
	cp $(RAILS_LIBSRC)/`basename $@` $@