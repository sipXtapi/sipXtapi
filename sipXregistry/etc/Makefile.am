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
install-data-hook : \
	$(foreach etcfile,$(editetc_IN),$(DESTDIR)$(sysconfdir)/sipxpbx/$(etcfile)) \
	$(DESTDIR)$(SIPX_PROCDIR)/sipregistrar.process.xml

$(foreach etcfile,$(editetc_IN),$(etcfile).tmp) : %.tmp : %
	$(LocalizeSipXconfig) $< > $@

$(DESTDIR)$(sysconfdir)/sipxpbx/registrar-config.in: registrar-config.in.tmp
	if [ -f $@ ]; \
	then \
	  echo "    Using existing $@"; \
	  echo -n "        Checking mwi ..."; \
	  if grep 'SIP_REGISTRAR_HOOK_LIBRARY.MWI' $@ > /dev/null 2>&1 ; \
	  then \
	    echo " ok" ; \
	  else \
	  	 echo " adding new mwi hook"; \
	  	 echo "SIP_REGISTRAR_HOOK_LIBRARY.MWI : @SIPX_LIBDIR@/libRegistrarImpliedMWI.so" \
	  	 >> $@ ; \
	  	 perl -pi -e 's/SIP_REGISTRAR_MWI_SUBSCRIBE/SIP_REGISTRAR.MWI.UA/' $@; \
	  fi; \
	  echo -n "        Checking proxy port ..."; \
	  if grep 'SIP_REGISTRAR_PROXY_PORT' $@ > /dev/null 2>&1 ; \
	  then \
	  	 echo " ok" ; \
	  else \
	  	 echo " adding proxy port"; \
	  	 echo 'SIP_REGISTRAR_PROXY_PORT : $${PROXY_SERVER_SIP_PORT}' \
	  	 >> $@ ; \
	  fi; \
	  echo -n "        Checking for authproxy ..."; \
	  if grep 'SIP_REGISTRAR_AUTH_PROXY' $@ > /dev/null 2>&1 ; \
	  then \
	  	 echo " ok" ; \
	  else \
	  	 echo " adding authproxy"; \
	  	 echo 'SIP_REGISTRAR_AUTH_PROXY : $${AUTH_PROXY_SERVER_SIP_SRV_OR_HOSTPORT}' \
	  	 >> $@ ; \
	  fi; \
   else \
	  echo ""; echo "Installing default $@"; \
	  $(INSTALL) -D -m 644 $< $@; \
	fi

$(DESTDIR)$(sysconfdir)/sipxpbx/fallbackrules.xml.in: fallbackrules.xml.in.tmp
	  if [ -f $@ ]; \
	  then \
		echo "    Using existing $@"; \
		echo "        Checking/updating schema for $@"; \
		perl -pi \
			-e 's|<mappings>|<mappings xmlns="http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00">|' \
			$@; \
	  else \
		 echo ""; echo "Installing default $@"; \
		 $(INSTALL) -D -m 644 $< $@; \
	  fi

$(DESTDIR)$(sysconfdir)/sipxpbx/mappingrules.xml.in: mappingrules.xml.in.tmp
	  if [ -f $@ ]; \
	  then \
		echo "    Using existing $@"; \
		echo "        Checking/updating schema for $@"; \
		perl -pi \
			-e 's|<mappings>|<mappings xmlns="http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00">|' \
			$@; \
	  else \
		 echo ""; echo "Installing default $@"; \
		 $(INSTALL) -D -m 644 $< $@; \
	  fi
