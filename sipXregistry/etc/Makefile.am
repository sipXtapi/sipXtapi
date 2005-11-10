## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

INSTALL=@INSTALL@

# Files to be installed if they do not already exist.
# But if they already exist, the current contents may need to be edited.
editetc_IN = \
    registrar-config.in \
    mappingrules.xml.in \
    fallbackrules.xml.in

example_sipdb = \
	huntgroup.xml \
	alias.xml

EXTRA_DIST=\
  $(editetc_IN) \
  $(example_sipdb)

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

editsipdb_IN = \
	huntgroup.xml \
	alias.xml

.PHONY: editsipdb
editsipdb :
	@for f in $(editsipdb_IN); \
	do \
	  target=`basename $$f`; \
	  tgtdir=`dirname @SIPX_DBDIR@/$$target`; \
	  if [ -f $(DESTDIR)@SIPX_DBDIR@/$$target ]; \
	  then \
		echo "    Checking/updating schema for existing @SIPX_DBDIR@/$$target"; \
		case $$target in \
			huntgroup.xml) \
				perl -pi \
				-e 's|<items type="huntgroup">|<items type="huntgroup" xmlns="http://www.sipfoundry.org/sipX/schema/xml/huntgroup-00-00">|' \
				$(DESTDIR)@SIPX_DBDIR@/$$target; \
				;; \
			alias.xml) \
				perl -pi \
				-e 's|<items type="alias">|<items type="alias" xmlns="http://www.sipfoundry.org/sipX/schema/xml/alias-00-00">|' \
				$(DESTDIR)@SIPX_DBDIR@/$$target; \
				;; \
		esac; \
	  fi; \
	done

.PHONY: example_sipdb
example_sipdb: 
	@for f in $(example_sipdb); \
	do \
		target=`basename $$f`; \
		tgtdir=`dirname @SIPX_DBDIR@/$$target`; \
		if [ ! -f $(DESTDIR)@SIPX_DBDIR@/$$target ]; \
		then \
			echo "    Installing empty example @SIPX_DBDIR@/$$target"; \
			$(INSTALL) -D -m 644 $(srcdir)/$$target $(DESTDIR)@SIPX_DBDIR@/$$target; \
		else \
			echo "    Using existing @SIPX_DBDIR@/$$target"; \
		fi; \
	done

install-data-hook : editetc editsipdb example_sipdb


