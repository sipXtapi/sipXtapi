
ACLOCAL_AMFLAGS = -I m4

include $(top_srcdir)/config/sipXcommon.am.inc

EXTRA_DIST = \
    config/sipXpkgtop.mak \
    config/sipX-config.in \
    config/sipX-buildstamp.cpp.in \
    config/sipX-buildstamp.h.in \
    config/sipXprojtop.mak \
    config/sipXcommon.am.inc

unconditional_SUBDIRS = \
  sipXportLib \
  sipXsdpLib \
  sipXtackLib \
  sipXmediaLib \
  sipXmediaAdapterLib \
  sipXcallLib

include $(top_srcdir)/config/sipXpkgtop.mak

SUBDIRS = $(unconditional_SUBDIRS) $(doc_SUBDIRS)


