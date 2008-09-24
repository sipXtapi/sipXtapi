SUBDIRS =

INCLUDES = -I$(top_srcdir)/include 

CheckVMaildir = .
CheckVMail_PROGRAMS = CheckVMail

CheckVMail_LDFLAGS = @PCRE_LDFLAGS@

CheckVMail_LDADD = \
    @SIPXPORT_STATIC_LIBS@ \
    @SIPXTACK_STATIC_LIBS@ \
    @SIPXMEDIA_STATIC_LIBS@ \
    @SIPXMEDIA_MP_STATIC_LIBS@ \
    @CA_LIBS@  \
    @PCRE_LIBS@ \
    @SSL_LIBS@ \
    @RT_LIBS@ \
    -lresolv \
    $(top_builddir)/src/.libs/libsipXcall.a

CheckVMail_SOURCES = CheckVMail.cpp
