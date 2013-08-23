## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/sipXtackLib/include -I$(top_srcdir)/sipXsdpLib/include -I$(top_srcdir)/sipXportLib/include

nobase_pkgincludeHEADERS_INSTALL = $(INSTALL) -D -p -c -m 644

nobase_pkginclude_HEADERS =  \
    mi/CpMediaInterface.h \
    mi/CpMediaInterfaceFactoryImpl.h \
    mi/CpMediaInterfaceFactoryFactory.h \
    mi/CpMediaInterfaceFactory.h \
    mi/IMediaSocket.h \
    mi/MiNotification.h \
    mi/MiIntNotf.h \
    mi/MiDtmfNotf.h \
    mi/MiProgressNotf.h \
    mi/MiRtpStreamActivityNotf.h \
    mi/MiStringNotf.h 
    
noinst_LTLIBRARIES = libmediainterface.la

libmediainterface_la_LIBADD =  \
	$(top_builddir)/sipXmediaLib/src/libsipXmedia.la

libmediainterface_la_SOURCES =  \
    CpMediaInterface.cpp \
    CpMediaInterfaceFactoryImpl.cpp \
    CpMediaInterfaceFactory.cpp \
    MiNotification.cpp \
    MiIntNotf.cpp \
    MiDtmfNotf.cpp \
    MiProgressNotf.cpp \
    MiRtpStreamActivityNotf.cpp \
    MiStringNotf.cpp \
    mi/CpMediaInterface.h \
    mi/CpMediaInterfaceFactoryImpl.h \
    mi/CpMediaInterfaceFactoryFactory.h \
    mi/CpMediaInterfaceFactory.h



