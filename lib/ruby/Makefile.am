include ../meta/libsrc.mak
RUBY_LIBSRC = $(LIBSRC)/ruby

RUBY_VER = 1.8.4
RUBY_REL = 1

SRPM = $(RUBY_LIBSRC)/ruby-$(RUBY_VER)-$(RUBY_REL).c4.src.rpm
RPMS = \
	ruby-$(RUBY_VER)-$(RUBY_REL).c4.$(RPM_TARGET_ARCH).rpm

all : $(RPMS)

$(SRPM) :
	$(srcdir)/config/download-file $@ $(RUBY_RPM_URL)/SRPMS/`basename $@`

.PHONY:build-rpms
build-rpms : $(SRPM)
	rpmbuild --rebuild $(SRPM)

$(RPMS) : build-rpms
	cp $(RPMBUILD_TOPDIR)/RPMS/`basename $@` $@
