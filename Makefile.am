
SUBDIRS = . bin neoconf common profilegenerator profilepublisher profilewriter sipagent-jni doc

EXTRA_DIST = \
    config/sipX-config.in \
    ant-targets.am \
    ant-targets.xml \
    subant-targets.xml \
    acsite.m4 \
    CONTRIBUTORS \
    LICENSE \
    config.properties.in \
    common \
    neoconf \
    sipxconfig.spec \
    profilegenerator \
    profilewriter \
    profilepublisher \
    profilepub_sipagent.dsp \
    lib

# See doc/Makefile.am why user has to explicitly type
# 'make doc'
.PHONY : doc
doc :
	cd doc && $(MAKE) $(AM_MAKEFLAGS)

## Search and replace helpful make/conf variables
edit = sed \
        -e 's,@abs_srcdir\@,@abs_srcdir@,g' \
        -e 's,@abs_builddir\@,@abs_builddir@,g' \
        -e 's,@CLOVER_HOME\@,@CLOVER_HOME@,g' \
        -e 's,@JAVA\@,@JAVA@,g' \
        -e 's,@JAVAC_OPTIMIZED\@,@JAVAC_OPTIMIZED@,g' \
        -e 's,@JAVAC_DEBUG\@,@JAVAC_DEBUG@,g' \
        -e 's,@PACKAGE\@,@PACKAGE@,g' \
        -e 's,@VERSION\@,@VERSION@,g' \
        -e 's,@JAVA_HOME\@,@JAVA_HOME@,g' \
        -e 's,@JBOSS_HOME\@,@JBOSS_HOME@,g' \
        -e 's,@SPRING_HOME\@,@SPRING_HOME@,g' \
        -e 's,@SIPXCONFIG_LIBDIR\@,@SIPXCONFIG_LIBDIR@,g' \
        -e 's,@SIPX_CONFIGPHONEDIR\@,@SIPX_CONFIGPHONEDIR@,g' \
        -e 's,@SIPX_CONFDIR\@,@SIPX_CONFDIR@,g' \
        -e 's,@SIPX_DATADIR\@,@SIPX_DATADIR@,g' \
        -e 's,@SIPXPBXUSER\@,@SIPXPBXUSER@,g' \
        -e 's,@profiledir\@,@profiledir@,g' \
        -e 's,@htmldir\@,@htmldir@,g' \
        -e 's,@localstatedir\@,$(localstatedir),g' \
        -e 's,@SIPX_LOGDIR\@,@SIPX_LOGDIR@,g' \
        -e 's,@SIPX_RUNDIR\@,@SIPX_RUNDIR@,g' \
        -e 's,@SIPX_TMPDIR\@,@SIPX_TMPDIR@,g' \
        -e 's,@libdir\@,$(libdir),g' \
        -e 's,@bindir\@,$(bindir),g'


# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile $(top_srcdir)/top.build.dir
	$(edit) $(srcdir)/config.properties.in > $(top_builddir)/config.properties

# This sets the default build dir to the last directory that called
# configure shell script.  This allows 'ant' to be called from any directory
$(top_srcdir)/top.build.dir :
	echo "top.build.dir="`pwd` > $(top_srcdir)/top.build.dir

# Alternative is to do svn export, but it's very handy to be able to 
# create a dist tarball from a working svn checkout
dist-hook : sipxconfig.spec
	rm -rf `find $(distdir) -type d -name .svn`
	cp sipxconfig.spec $(distdir)


# Override default tarball creation, need to support paths > 99 chars
# change tar optoions: "chof"  to "chf", implications tarball is not
# compatible on legacy systems.  See 'man tar' for more info
# Other dist types will have same problem, but not orerriding yet as we do
# support them at this time nor do I have time to check/maintain them, yet.
dist dist-all : distdir
	$(AMTAR) chf - $(distdir) | GZIP=$(GZIP_ENV) gzip -c >$(distdir).tar.gz
	$(am__remove_distdir)

.PHONY : rpm
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
rpm : clean dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	mv -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	mv -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .

# RPM Spec file
# Extract the options to ./configure from config.log and propagate them into the .spec file.
sipxconfig.spec : sipxconfig.spec.in
	V="$$( sed -e '/^ *\$$ .*\/configure/!d' -e 's/^.*\/configure *//' config.log )" ; \
	$(edit) -e "s#@CONFIGURE_OPTIONS@#$$V#" \
		$(srcdir)/sipxconfig.spec.in > sipxconfig.spec
