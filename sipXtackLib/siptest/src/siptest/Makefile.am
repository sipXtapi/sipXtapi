## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(srcdir)/../../include

bin_PROGRAMS = siptest
siptest_LDFLAGS = $(top_builddir)/src/libsipXtack.la ${GLIB_LIBS}
siptest_CXXFLAGS = $(GLIB_CFLAGS)
siptest_SOURCES =  \
    AuthCommand.cpp \
    Command.cpp \
    CommandMsgProcessor.cpp \
    CommandProcessor.cpp \
    ExitCommand.cpp \
    HelpCommand.cpp \
    HistoryCommand.cpp \
    HttpGetCommand.cpp \
    RespondCommand.cpp \
    RespondTemplate.cpp \
    sipdrive.cpp \
    SipLogCommand.cpp \
    SipLSendCommand.cpp \
    SipSendCommand.cpp
