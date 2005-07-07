## Process this file with automake to produce Makefile.in

INCLUDES = -I$(srcdir) @APACHE2_CXXFLAGS@

# Install the module using the Apache tool.
# Ensure the target directory exists first.
install-exec-hook:
	test -d $(DESTDIR)@APACHE2_MOD@ || @INSTALL@ -d -m 755 $(DESTDIR)@APACHE2_MOD@
	@APACHE2_APXS@ -i -n libmod_sipxauth \
		-S LIBEXECDIR=$(DESTDIR)@APACHE2_MOD@ \
		libmod_sipxauth.la

# Order automake to build the .la, but not install it.
noinst_LTLIBRARIES = libmod_sipxauth.la

# Add special options to build a shared library.
libmod_sipxauth_la_LDFLAGS = $(GLIB_LIBS) -module -avoid-version \
	-rpath $(DESTDIR)@APACHE2_MOD@
libmod_sipxauth_la_CXXFLAGS = -DSIPXCHANGE_USERNAME=\"@SIPXPBXUSER@\" \
	$(GLIB_CFLAGS) -DSIPX_LOGDIR=\"@SIPX_LOGDIR@\"
libmod_sipxauth_la_SOURCES = \
    SIPXAuthHandler.cpp

noinst_HEADERS = \
	SIPXAuthHandler.h
