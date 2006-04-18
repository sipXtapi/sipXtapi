include $(top_builddir)/config/sipXcommon.mak

INSTALL = @INSTALL@

rake=$(RAKE) builddir="$$builddir"
GEM_LIB_DIR=$(libdir)/ruby/gems/1.8
callresolver_dir=$(datadir)/sipxpbx/lib

clean :
	builddir=$(srcdir); \
	cd $(srcdir) && rake clobber

if IS_RPMBUILD
INSTALL_GEM = install-gem-for-rpm
else
INSTALL_GEM = install-gem
endif

install : $(INSTALL_GEM) 

.PHONY: package-gem
package-gem :
	pushd .; rm -rf $(srcdir)/pkg; \
	cd $(srcdir) && rake callresolver_version=1.0.0 builddir=. package; \
	popd

.PHONY: install-gem 
install-gem : package-gem
	$(mkdir_p) $(GEM_LIB_DIR)
	gem install \
	  --install-dir $(GEM_LIB_DIR) \
	  --local $(srcdir)/pkg/sipxcallresolver
	mv $(srcdir)/pkg/sipxcallresolver*.gem $(top_builddir)/src/cdr
	rm -rf $(srcdir)/pkg

# gems are copied into installation and will be installed on host machine
# later
.PHONY: install-gem-for-rpm
install-gem-for-rpm : package-gem
	echo "Installing call resolver gem"
	$(INSTALL) -d -m755 $(DESTDIR)$(callresolver_dir)
	$(INSTALL) pkg/sipxcallresolver*.gem $(DESTDIR)$(callresolver_dir)
