include $(top_builddir)/config/sipXcommon.mak

include $(srcdir)/config/sipXprojtop.mak
# Append to global search/replace with _very_ project specific search/replaces
edit = $(LocalizeSipXconfig) \
    -e 's,@CLOVER_JAR\@,@CLOVER_JAR@,g' \
    -e 's,@JBOSS_HOME\@,@JBOSS_HOME@,g' \
    -e 's,@JBOSS_SERVER_CONF\@,@JBOSS_SERVER_CONF@,g' \
    -e 's,@SIPFOUNDRY_ORG\@,@SIPFOUNDRY_ORG@,g'


if SIPAGENT
SIPAGENT_OPT = sipagent-jni
endif

SUBDIRS = . bin common neoconf profilegenerator profilepublisher profilewriter $(SIPAGENT_OPT) web $(doc_SUBDIRS)
RELEASE = 2

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.mak.in \
    ant-targets.am \
    ant-targets.xml \
    subant-targets.xml \
    acsite.m4 \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN-VERSION \
    LICENSE \
    config.properties.in \
    common \
    neoconf \
    sipxconfig.spec \
    web \
    profilegenerator \
    profilewriter \
    profilepublisher \
    profilepub_sipagent.dsp \
    lib

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
# Get the revision number
if USE_BLDNO
BUILDPARM=--define="buildno 0.$(shell cat $(srcdir)/SVN-VERSION)"
else
BUILDPARM=--define="buildno $(RELEASE)"
endif

rpm : clean dist
	rpmbuild -ta $(BUILDPARM) $(PACKAGE)-$(VERSION).tar.gz
	mv -f $(RPMBUILD_TOPDIR)/SRPMS/$(PACKAGE)-$(VERSION)-*.rpm .
	mv -f $(RPMBUILD_TOPDIR)/RPMS/*/$(PACKAGE)*-$(VERSION)-*.rpm .
	md5sum $(PACKAGE)-$(VERSION).tar.gz >$(PACKAGE)-$(VERSION).tar.gz.md5

# RPM Spec file
# Extract the options to ./configure from config.log and propagate them into the .spec file.
sipxconfig.spec : sipxconfig.spec.in
	V="$$( sed -e '/^ *\$$ .*\/configure/!d' -e 's/^.*\/configure *//' config.log )" ; \
	$(LocalizeSipXconfig) -e "s#@CONFIGURE_OPTIONS@#$$V#" \
		$(srcdir)/sipxconfig.spec.in > sipxconfig.spec
