
SUBDIRS = database

INSTALL=@INSTALL@

include $(top_builddir)/config/sipXcommon.mak

cfgfiles_IN = \
    authproxy-config.in \
    authrules.xml.in \
    callresolver-config.in \
    proxy-config.in \
    forwardingrules.xml.in

EXTRA_DIST = \
    $(cfgfiles_IN)

install-exec-hook : $(foreach file,$(cfgfiles_IN),$(DESTDIR)/$(sysconfdir)/sipxpbx/$(file)) \
						  upgrade-auth-rules

$(foreach file,$(cfgfiles_IN),$(DESTDIR)/$(sysconfdir)/sipxpbx/$(file)) : $(DESTDIR)/$(sysconfdir)/sipxpbx/% : %
	if ! test -f $@; \
	then \
		echo "Installing default $@"; \
		$(INSTALL) -D -m 644 $< $@; \
	else \
		diff $< $@ > /dev/null 2>&1 \
		|| (  echo "Using existing $@ - new file is $@.NEW" \
		    ; $(INSTALL) -D -m 644 $< $@.NEW; \
			); \
	fi

.PHONY: upgrade-auth-rules
upgrade-auth-rules:
	echo "        Checking/updating schema for $(sysconfdir)/sipxpbx/authrules.xml.in"
	perl -pi \
	-e 's|<mappings>|<mappings xmlns="http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00">|' \
	$(DESTDIR)$(sysconfdir)/sipxpbx/authrules.xml.in
