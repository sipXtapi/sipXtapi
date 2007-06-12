RAILS_LIBSRC=$(LIBSRC)/ruby-rails

RAILS = \
	rubygem-actionmailer-1.3.2-1.fc6.noarch.rpm \
	rubygem-actionpack-1.13.2-1.fc6.noarch.rpm \
	rubygem-actionwebservice-1.2.2-1.fc6.noarch.rpm \
	rubygem-activerecord-1.15.2-1.fc6.noarch.rpm \
	rubygem-activesupport-1.4.1-1.fc6.noarch.rpm \
	rubygem-rails-1.2.2-1.fc6.noarch.rpm \
	rubygem-rake-0.7.1-1.fc6.noarch.rpm

DOWNLOAD = $(foreach file,$(RAILS),$(RAILS_LIBSRC)/$(file))
RPMS = $(foreach file,$(RAILS),$(DEST_RPM)/$(file))

all : $(RPMS)

$(DOWNLOAD) :
	$(DOWNLOAD_FILE) $@ $(RUBY_AUX_RPMS_URL)/`basename $@`

$(RPMS) : $(DOWNLOAD)
	cp $(RAILS_LIBSRC)/`basename $@` $@
