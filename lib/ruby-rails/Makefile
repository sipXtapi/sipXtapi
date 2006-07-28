# simply download prebuilt from dlutter :)

# type  "make LIBSRC=XXX" to override this location, normal make syntax
LIBSRC=~/libsrc
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
	-mkdir $(RAILS_LIBSRC)
	wget --output-document=$@ \
	  http://people.redhat.com/dlutter/yum/RPMS/`basename $@`

$(RAILS_RPMS) : $(RAILS_RPMS_DOWNLOAD)
	cp $(RAILS_LIBSRC)/$@ $@
