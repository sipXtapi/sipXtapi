AM_CFLAGS = -DCOMPANY_DOMAIN=L\"pingtel.com\" $(GLIB_CFLAGS)
AM_CXXFLAGS = -DCOMPANY_DOMAIN=L\"pingtel.com\" $(GLIB_CFLAGS)

noinst_LIBRARIES = libOSBclient.a

bin_PROGRAMS = sipXvxml

libOSBclient_a_SOURCES = \
	ivr/IvrUtilTask.cpp \
	ivr/IvrTelListener.cpp \
	api/OSBclient.cpp \
	api/SBclientUtils.cpp \
	api/SBobject.cpp

sipXvxml_LDFLAGS = $(GLIB_LIBS)
sipXvxml_SOURCES = \
	ivr/ConfigFile.cpp \
	ivr/IvrCallListener.cpp \
	ivr/IvrDtmfListener.cpp \
	ivr/clientMain.cpp

sipXvxml_LDADD = \
	-L../client -L../inet -L../jsi -L../log -L../prompt -L../rec -L../tel \
	-L../trd -L../value -L../VXI -lOSBclient -lOSBinet -lOSBjsi -lOSBlog \
	-lOSBprompt -lOSBrec -lOSBtel -lOSBtrd -lOSBvalue -lVXI \
	-L../../js/ -ljavascript -lresolv 

noinst_HEADERS = \
	ivr/clientMain.h \
	ivr/ConfigFile.cpp \
	ivr/ConfigFile.h \
	ivr/IvrCallListener.cpp \
	ivr/IvrCallListener.h \
	ivr/IvrDtmfListener.cpp \
	ivr/IvrDtmfListener.h \
	ivr/IvrTelListener.cpp \
	ivr/IvrTelListener.h \
	ivr/IvrUtilTask.cpp \
	ivr/IvrUtilTask.h
