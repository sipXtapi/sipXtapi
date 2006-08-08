# simply download prebuilt from dlutter :)

include ../meta/libsrc.mak

GEMS_LIBSRC=$(LIBSRC)/ruby-gems

GEMS_RPM = ruby-gems-0.8.11-2.noarch.rpm

all : $(GEMS_RPM)

$(GEMS_LIBSRC)/$(GEMS_RPM) :
	../meta/download-file $@ $(RUBY_AUX_RPMS_URL)/RPMS/`basename $@`

$(GEMS_RPM) : $(GEMS_LIBSRC)/$(GEMS_RPM)
	cp $(GEMS_LIBSRC)/$(GEMS_RPM) .
