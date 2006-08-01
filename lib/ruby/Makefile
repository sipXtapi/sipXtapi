include ../meta/libsrc.mak
RUBY_LIBSRC = $(LIBSRC)/ruby

RUBY_VER = 1.8.4
RUBY_REL = 1

RUBY_SRPM = $(RUBY_LIBSRC)/ruby-$(RUBY_VER)-$(RUBY_REL).c4.src.rpm
RUBY_RPM = ruby-$(RUBY_VER)-$(RUBY_REL).c4.$(RPM_TARGET_ARCH).rpm

all : $(RUBY_RPM)

$(RUBY_SRPM) :
	../meta/download-file $@ $(RUBY_RPM_URL)/SRPMS/`basename $@`

$(RUBY_RPM) : $(RUBY_SRPM)
	rpmbuild --rebuild $(RUBY_SRPM)
	cp $(RPMBUILD_TOPDIR)/RPMS/$(RPM_TARGET_ARCH)/*.rpm .
