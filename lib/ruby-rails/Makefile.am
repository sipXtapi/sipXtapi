# simply download prebuilt from dlutter :)

include ../meta/libsrc.mak

RAILS_LIBSRC=$(LIBSRC)/ruby-rails

RAILS_RPMS = \
	ruby-gems-actionmailer-1.1.5-1.noarch.rpm \
	ruby-gems-actionpack-1.11.2-1.noarch.rpm \
	ruby-gems-actionwebservice-1.0.0-1.noarch.rpm \
	ruby-gems-activerecord-1.13.2-1.noarch.rpm \
	ruby-gems-activesupport-1.2.5-1.noarch.rpm \
	ruby-gems-rails-1.0.0-1.noarch.rpm \
	ruby-gems-rake-0.7.0-1.noarch.rpm

RAILS_RPMS_DOWNLOAD = $(foreach file,$(RAILS_RPMS),$(RAILS_LIBSRC)/$(file))

all : $(RAILS_RPMS)

$(RAILS_RPMS_DOWNLOAD) :
	../meta/download-file $@ $(RUBY_AUX_RPMS_URL)/RPMS/`basename $@`

$(RAILS_RPMS) : $(RAILS_RPMS_DOWNLOAD)
	cp $(RAILS_LIBSRC)/$@ $@