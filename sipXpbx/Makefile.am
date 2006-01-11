## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

include $(srcdir)/config/sipXprojtop.mak

SUBDIRS = . \
    include \
    src \
    bin \
    etc \
    sipXpark \
    sipXpresence \
    sipXdialog \
    meta \
    $(doc_SUBDIRS)

INSTALL = @INSTALL@

EXTRA_DIST = \
    BUILDSTAMP \
    CONTRIBUTORS \
    LICENSE \
    SVN-EXPORT-VERSION \
    bin/httpd-sipxchange-config.sh.in \
    config/apache_version.awk \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipX-config.in \
    config/sipXcommon.mak.in \
    config/sipXprojtop.mak \
    sipxpbx.spec

install-exec-hook :
	$(LocalizeSipXconfig) $(srcdir)/bin/httpd-sipxchange-config.sh.in > httpd-sipxchange-config.sh
	$(INSTALL) -D -m 755 httpd-sipxchange-config.sh $(DESTDIR)$(bindir)/httpd-sipxchange-config.sh

install-data-hook :
	$(INSTALL) -d $(DESTDIR)@wwwdir@/doc
	$(INSTALL) -d $(DESTDIR)@wwwdir@/conf

dist-hook :
	rm -rf $(distdir)/src/cgilib $(distdir)/include/cgilib

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
