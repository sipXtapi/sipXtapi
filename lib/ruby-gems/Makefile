# simply download prebuilt from dlutter :)

# type  "make LIBSRC=XXX" to override this location, normal make syntax
LIBSRC=~/libsrc
GEMS_LIBSRC=$(LIBSRC)/ruby-gems

GEMS_RPM = ruby-gems-0.8.11-2.noarch.rpm

all : $(GEMS_RPM)

$(GEMS_LIBSRC)/$(GEMS_RPM) :
	-mkdir $(GEMS_LIBSRC)
	wget --output-document=$@ \
	  http://people.redhat.com/dlutter/yum/RPMS/`basename $@`

$(GEMS_RPM) : $(GEMS_LIBSRC)/$(GEMS_RPM)
	cp $(GEMS_LIBSRC)/$(GEMS_RPM) .
