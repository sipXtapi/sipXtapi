## Process this file with automake to produce Makefile.in

include $(top_builddir)/config/sipXcommon.mak
include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = . src bin meta $(doc_SUBDIRS)
RELEASE = 1

INSTALL=@INSTALL@

editdata_IN = \
    etc/authproxy-config.in \
    etc/authrules.xml.in \
    etc/proxy-config.in \
    etc/forwardingrules.xml.in

sql_schemas = \
    etc/database/schema.sql

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
    CONTRIBUTORS \
    BUILDSTAMP \
	 SVN-EXPORT-VERSION \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.mak.in \
    $(editdata_IN) \
    $(sql_schemas) \
    sipxproxy.spec

install-exec-hook : editdata

install-data-hook :
	$(INSTALL) -d -m 755 $(DESTDIR)$(sysconfdir)/sipxpbx/cdr
	@for sql_file in $(sql_schemas); \
  do \
    echo "Installing CDR schema"; \
    target=`basename $$sql_file`;\
    $(INSTALL) -D -m 644 $(srcdir)/$$sql_file $(DESTDIR)$(sysconfdir)/sipxpbx/cdr/$$target; \
  done

.PHONY : editdata

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
