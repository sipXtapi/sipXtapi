SUBDIRS =

INCLUDES = -I$(top_srcdir)/sipXcallLib/include 

CheckVMaildir = .
CheckVMail_PROGRAMS = CheckVMail

CheckVMail_LDFLAGS = @PCRE_LDFLAGS@

CheckVMail_LDADD = \
    @SIPXPORT_STATIC_LIBS@ \
    @SIPXTACK_STATIC_LIBS@ \
    @SIPXMEDIA_STATIC_LIBS@ \
    @SIPXMEDIA_MP_STATIC_LIBS@ \
    @CA_LIBS@  \
    @RT_LIBS@ \
    -lresolv \
    $(top_builddir)/sipXcallLib/src/.libs/libsipXcall.a

CheckVMail_SOURCES = CheckVMail.cpp
