## Process this file with automake to produce Makefile.in

include $(top_builddir)/config/sipXcommon.mak

unconditional_SUBDIRS = . src bin

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)
RELEASE = 1

INSTALL=@INSTALL@

editdata_IN = \
    etc/authproxy-config.in \
    etc/authrules.xml.in \
    etc/proxy-config.in \
    etc/forwardingrules.xml.in

editdata :
	@for cfgfile in $(editdata_IN); \
	do \
		target=`basename $$cfgfile`; \
		$(LocalizeSipXconfig) $(srcdir)/$$cfgfile > $$target.tmp; \
		if ! test -f $(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
		then \
			echo ""; echo "Installing default $(sysconfdir)/sipxpbx/$$target"; \
			$(INSTALL) -D -m 644 $$target.tmp $(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
		else \
	    	 echo ""; echo "Using existing $(sysconfdir)/sipxpbx/$$target - new file is at $$target.new"; \
			 $(INSTALL) -D -m 644 $$target.tmp $(DESTDIR)$(sysconfdir)/sipxpbx/$$target.new; \
			 case $$target in \
			   authrules.xml.in) \
				  echo "        Checking/updating schema for $(sysconfdir)/sipxpbx/$$target"; \
				  perl -pi \
				    -e 's|<mappings>|<mappings xmlns="http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00">|' \
				    $(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
				  ;; \
		    esac; \
	   fi; \
	done; \
	echo ""

EXTRA_DIST = \
    config/localize-sipX.sh.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipX-config.in \
    config/sipXcommon.mak \
    config/sipXprojtop.mak \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN-EXPORT-VERSION \
    $(editdata_IN) \
    sipxproxy.spec

install-exec-hook : editdata

.PHONY : editdata

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
