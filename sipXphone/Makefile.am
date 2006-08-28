## Process this file with automake to produce Makefile.in
include $(top_srcddir)/config/subdir.am

EXTRA_DIST = \
    ant-targets.xml \
    config.properties.in \
    doc/license

SUBDIRS = \
    src \
    include \
    javacore \
    softphone

phonelibdir = @SIPXPHONELIB@
dist_phonelib_DATA = \
    lib/jndi1.2.1.jar \
    lib/jtapi13.jar \
    lib/ldap.jar \
    lib/oromatch.jar

config.properties : $(srcdir)/config.properties.in
	$(LocalizeSipXconfig) $(srcdir)/config.properties.in > $(top_builddir)/config.properties


# Alternative is to do svn export, but it's very handy to be able to 
# create a dist tarball from a working svn checkout
dist-hook:
	rm -rf `find $(distdir) -type d -name .svn`

# Override default tarball creation, need to support paths > 99 chars
# change tar options: "chof"  to "chf", implications tarball is not
# compatible on legacy systems.  See 'man tar' for more info
# Other dist types will have same problem, but not overriding yet as we do
# support them at this time nor do I have time to check/maintain them, yet.
dist dist-all: distdir
	$(AMTAR) chf - $(distdir) | GZIP=$(GZIP_ENV) gzip -c >$(distdir).tar.gz
	$(am__remove_distdir)

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
