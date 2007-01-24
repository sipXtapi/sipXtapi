
GEMS_LIBSRC=$(LIBSRC)/ruby-gems
VER = 0.8.11
REL = 2

RPM = ruby-gems-$(VER)-$(REL).noarch.rpm

all : $(DEST_RPM)/$(RPM)

# simply download, no need to patch/recompile
$(GEMS_LIBSRC)/$(RPM) :
	$(DOWNLOAD_FILE) $@ $(RUBY_AUX_RPMS_URL)/RPMS/`basename $@`

# simply download, no need to patch/recompile
$(DEST_RPM)/$(RPM) : $(GEMS_LIBSRC)/$(RPM)
	cp $< $@
