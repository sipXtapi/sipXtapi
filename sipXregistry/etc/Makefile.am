## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am


# Files to be installed if they do not already exist.
# But if they already exist, the current contents may need to be edited.
editetc_IN = \
    registrar-config.in \
    mappingrules.xml.in \
    fallbackrules.xml.in

EXTRA_DIST = \
  sipregistrar.process.xml.in \
  $(editetc_IN) 

# Install files in $(editetc_IN), or if they already exist, edit
# them to be compatible with the current code.
.PHONY: editetc
editetc :
	@for f in $(editetc_IN); \
	do \
	  target=`basename $$f`; \
	  $(LocalizeSipXconfig) $(srcdir)/$$f > $$target.tmp; \
	  tgtdir=`dirname $(sysconfdir)/sipxpbx/$$target`; \
	  if [ -f $(DESTDIR)$(sysconfdir)/sipxpbx/$$target ]; \
	  then \
		echo "    Using existing $(sysconfdir)/sipxpbx/$$target"; \
		case $$target in \
			registrar-config.in) \
				echo -n "        Checking mwi in $(sysconfdir)/sipxpbx/$$target ..."; \
				if grep 'SIP_REGISTRAR_HOOK_LIBRARY.MWI' $(sysconfdir)/sipxpbx/$$target > /dev/null; \
				then \
					echo " ok" ; \
				else \
					echo " adding"; \
					echo "SIP_REGISTRAR_HOOK_LIBRARY.MWI : @SIPX_LIBDIR@/libRegistrarImpliedMWI.so" \
					>> $(sysconfdir)/sipxpbx/$$target ; \
					perl -pi \
					-e 's/SIP_REGISTRAR_MWI_SUBSCRIBE/SIP_REGISTRAR.MWI.UA/' \
					$(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
				fi; \
				echo -n "        Checking proxy port in $(sysconfdir)/sipxpbx/$$target ..."; \
				if grep 'SIP_REGISTRAR_PROXY_PORT' $(sysconfdir)/sipxpbx/$$target > /dev/null; \
				then \
					echo " ok" ; \
				else \
					echo " adding"; \
					echo 'SIP_REGISTRAR_PROXY_PORT : $${PROXY_SERVER_SIP_PORT}' \
					>> $(sysconfdir)/sipxpbx/$$target ; \
				fi; \
				;; \
			fallbackrules.xml.in|mappingrules.xml.in) \
				echo "        Checking/updating schema for $(sysconfdir)/sipxpbx/$$target"; \
				perl -pi \
				-e 's|<mappings>|<mappings xmlns="http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00">|' \
				$(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
				;; \
		esac; \
	  else \
		echo ""; echo "Installing default $(sysconfdir)/sipxpbx/$$target"; \
		$(INSTALL) -D -m 644 $$target.tmp $(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
	  fi; \
	done

install-data-hook : \
	editetc \
	$(DESTDIR)$(SIPX_PROCDIR)/sipregistrar.process.xml
