
GEMS_LIBSRC=$(LIBSRC)/rubygems
VER = 0.9.1
REL = 1

RPM = $(DEST_RPM)/rubygems-$(VER)-$(REL).fc6.noarch.rpm

all : $(RPM)

# simply download, no need to patch/recompile
$(RPM) :
	$(DOWNLOAD_FILE) $@ $(RUBY_AUX_RPMS_URL)/`basename $@`
