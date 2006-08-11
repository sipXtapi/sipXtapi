
GEMS_LIBSRC=$(LIBSRC)/ruby-gems
VER = 0.8.11
REL = 2

RPM = $(DEST_RPM)/ruby-gems-$(VER)-$(REL).noarch.rpm

all : $(RPM)

# simply download, no need to patch/recompile
$(RPM) :
	$(srcdir)/config/download-file $@ $(RUBY_AUX_RPMS_URL)/RPMS/`basename $@`
