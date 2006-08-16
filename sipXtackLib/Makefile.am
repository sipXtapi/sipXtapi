## Process this file with automake to produce Makefile.in
include $(srcdir)/config/project.am

EXTRA_DIST = \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    CONTRIBUTORS \
    LICENSE \
    BUILDSTAMP \
    SVN_VERSION \
    ant-targets.xml \
    ant-targets.am \
    sipxtacklib.spec \
    lib/jdom-b10.jar \
    lib/junit.jar \
    config.properties.in \
    syslog2siptrace/siptrace-merge

if SIPVIEWER
SIPVIEWER_OPT = sipviewer
endif

if EXAMPLES
EXAMPLES_OPT = examples
endif

unconditional_SUBDIRS = . include src bin meta examples syslog2siptrace siplog2siptrace siptest $(SIPVIEWER_OPT) $(EXAMPLES_OPT)

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	$(LocalizeSipXconfig) $(srcdir)/config.properties.in > config.properties

DISTCLEANFILES = \
	$(CONFIG_DISTCLEANFILES)
