RUBY_LIBSRC = $(LIBSRC)/ruby

RUBY_VER = 1.8

ABI_RPM = \
	$(DEST_RPM)/ruby-abi-$(RUBY_VER)-1.noarch.rpm

all : $(ABI_RPM)

# Fullfills RUBY ABI requirement
# http://mail.madstop.com/pipermail/puppet-dev/2006-August/001503.html
$(ABI_RPM) : ruby-abi.spec
	rpmbuild -ba $(srcdir)/$<
	cp @RPMBUILD_TOPDIR@/RPMS/noarch/`basename $@` $@
