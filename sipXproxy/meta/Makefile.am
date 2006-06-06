include $(top_builddir)/config/sipXcommon.mak

gems_dir=$(datadir)/sipxpbx/lib
rails_gems=\
   actionmailer-1.1.5.gem \
   activesupport-1.2.5.gem \
   actionpack-1.11.2.gem \
   rails-1.0.0.gem \
   actionwebservice-1.0.0.gem \
   rake-0.7.0.gem \
   activerecord-1.13.2.gem


# Pristine copies of sipX 3.0 configuration files.  When no longer
# supporting upgrade from 3.0, you can delete this file and respective
# installation target
SIPX30_CONFIG = @PACKAGE@-3.0.tgz
PREVIOUS_CONFIG = $(DESTDIR)@SIPX_UPGRADEDIR@/@PACKAGE@-previous.tgz
EXTRA_DIST = \
	$(SIPX30_CONFIG) \
	$(rails_gems)

install-data-hook : $(PREVIOUS_CONFIG) $(foreach file,$(rails_gems),$(DESTDIR)$(gems_dir)/$(file))

$(PREVIOUS_CONFIG) : $(srcdir)/$(SIPX30_CONFIG)
	$(INSTALL) -D -m 644 $(srcdir)/$(SIPX30_CONFIG) $(PREVIOUS_CONFIG)

$(foreach file,$(rails_gems),$(DESTDIR)$(gems_dir)/$(file)): $(DESTDIR)$(gems_dir)/% : %
	$(INSTALL) -D -m 644 $< $@
	