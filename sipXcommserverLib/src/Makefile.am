## Process this file with automake to produce Makefile.in

SUBDIRS = . sipdbtest test

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/include/fastdb

lib_LTLIBRARIES = libsipXcommserver.la
libsipXcommserver_la_CXXFLAGS = \
	$(GLIB_CFLAGS) \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_DBDIR=\"@SIPX_DBDIR@\"

libsipXcommserver_la_SOURCES =  \
    sipdb/AliasDB.cpp \
    sipdb/AuthexceptionDB.cpp \
    sipdb/CredentialDB.cpp \
    sipdb/DialByNameDB.cpp \
    sipdb/ExtensionDB.cpp \
    sipdb/HuntgroupDB.cpp \
    sipdb/PermissionDB.cpp \
    sipdb/RegistrationDB.cpp \
    sipdb/ResultSet.cpp \
    sipdb/SIPDBManager.cpp \
    sipdb/SIPXAuthHelper.cpp \
    sipdb/SubscriptionDB.cpp \
    fastdb/cgistub.cpp \
    fastdb/class.cpp \
    fastdb/compiler.cpp \
    fastdb/container.cpp \
    fastdb/cursor.cpp \
    fastdb/database.cpp \
    fastdb/file.cpp \
    fastdb/hashtab.cpp \
    fastdb/localcli.cpp \
    fastdb/query.cpp \
    fastdb/repsock.cpp \
    fastdb/server.cpp \
    fastdb/symtab.cpp \
    fastdb/sync.cpp \
    fastdb/ttree.cpp \
    fastdb/unisock.cpp \
    fastdb/w32sock.cpp \
    fastdb/wwwapi.cpp \
    digitmaps/UrlMapping.cpp
