
include $(top_srcdir)/ant-targets.am

EXTRA_DIST = \
    doc/license \
    config.properties.in \
    common \
    profilegenerator \
    profilewriter \
    profilepublisher

## Search and replace helpful make/conf variables
edit = sed \
        -e 's,@abs_srcdir\@,@abs_srcdir@,g' \
        -e 's,@abs_builddir\@,@abs_builddir@,g' \
        -e 's,@JAVA\@,@JAVA@,g' \
        -e 's,@JAVAC_OPTIMIZED\@,@JAVAC_OPTIMIZED@,g' \
        -e 's,@JAVAC_DEBUG\@,@JAVAC_DEBUG@,g' \
        -e 's,@VERSION\@,@VERSION@,g' \
        -e 's,@JAVA_HOME\@,@JAVA_HOME@,g' \
        -e 's,@JBOSS_HOME\@,@JBOSS_HOME@,g' \
        -e 's,@TOMCAT_HOME\@,@TOMCAT_HOME@,g' \
        -e 's,@SIPXCONFIGLIB\@,@SIPXCONFIGLIB@,g' \
        -e 's,@SIPXPBXCONF\@,@SIPXPBXCONF@,g' \
        -e 's,@SIPXPBXDATA\@,@SIPXPBXDATA@,g' \
        -e 's,@profiledir\@,@profiledir@,g' \
        -e 's,@wwwdir\@,@wwwdir@,g' \
        -e 's,@localstatedir\@,$(localstatedir),g' \
        -e 's,@libdir\@,$(libdir),g' \
        -e 's,@bindir\@,$(bindir),g'

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	$(edit) $(srcdir)/config.properties.in > $(top_builddir)/config.properties
