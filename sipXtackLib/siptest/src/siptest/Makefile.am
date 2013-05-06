## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I$(top_srcdir)/sipXtackLib/include \
	-I$(srcdir)/../../include \
	-I$(top_srcdir)/sipXsdpLib/include \
	-I$(top_srcdir)/sipXportLib/include

bin_PROGRAMS = siptest

bin_SCRIPTS = subscribe-dialog-test

siptest_LDADD = $(top_builddir)/sipXtackLib/src/libsipXtack.la $(top_builddir)/sipXsdpLib/src/libsipXsdp.la $(top_builddir)/sipXportLib/src/libsipXport.la

siptest_SOURCES =  \
    AuthCommand.cpp \
    Command.cpp \
    CommandMsgProcessor.cpp \
    CommandProcessor.cpp \
    ExitCommand.cpp \
    HelpCommand.cpp \
    HistoryCommand.cpp \
    HttpGetCommand.cpp \
    RegisterCommand.cpp \
    RespondCommand.cpp \
    RespondTemplate.cpp \
    sipdrive.cpp \
    SipLogCommand.cpp \
    SipLSendCommand.cpp \
    SipSendCommand.cpp

EXTRA_DIST = \
    $(bin_SCRIPTS)
