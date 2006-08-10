include $(top_builddir)/config/sipXcommon.mak
include $(srcdir)/config/sipXprojtop.mak

# Append to global search/replace with _very_ project specific search/replaces
edit = $(LocalizeSipXconfig) \
    -e 's,@OPENSSL\@,@OPENSSL@,g' \
    -e 's,@CLOVER_JAR\@,@CLOVER_JAR@,g' \
    -e 's,@RUBY_BINDINGS_ENABLED\@,@RUBY_BINDINGS_ENABLED@,g' \
    -e 's,@RUBY\@,@RUBY@,g' \
    -e 's,@WSDL2RUBY\@,@WSDL2RUBY@,g' \
    -e 's,@SIPFOUNDRY_ORG\@,@SIPFOUNDRY_ORG@,g'


SUBDIRS = . bin neoconf web meta agent $(doc_SUBDIRS)

RPMS = $(RPM)

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.mak.in \
    ant-targets.am \
    ant-targets.xml \
    build.xml \
    dependencies.xml \
    subant-targets.xml \
    acsite.m4 \
    CONTRIBUTORS \
    BUILDSTAMP \
    SVN_VERSION \
    LICENSE \
    build.properties.in \
    lib.properties \
    neoconf \
    sipxconfig.spec \
    web \
    meta \
    agent \
    lib

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = build.properties

build.properties : build.properties.in Makefile _top.build.dir
	$(edit) $(srcdir)/build.properties.in > $(top_builddir)/build.properties

# This sets the default build dir to the last directory that called
# configure shell script.  This allows 'ant' to be called from any directory
# target name is not a file to force this make everytime
_top.build.dir :    
	echo "top.build.dir="`pwd` > $(top_srcdir)/top.build.dir

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
