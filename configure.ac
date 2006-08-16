AC_PREREQ(2.57)
AC_INIT(sipX, 0.0.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)

AC_CONFIG_SUBDIRS(sipXportLib)
AC_CONFIG_SUBDIRS(sipXtackLib)
AC_CONFIG_SUBDIRS(sipXmediaLib)
AC_CONFIG_SUBDIRS(sipXmediaAdapterLib)
AC_CONFIG_SUBDIRS(sipXcallLib)

AC_ARG_ENABLE(server, 
  AC_HELP_STRING([--disable-server], 
    [SIP servers, proxy, registrar, webui, mediaserver, etc. ]), 
     server=no, server=yes)

if test x$server = xyes
then
  AC_CONFIG_SUBDIRS(sipXcommserverLib)
  AC_CONFIG_SUBDIRS(sipXproxy)
  AC_CONFIG_SUBDIRS(sipXpublisher)
  AC_CONFIG_SUBDIRS(sipXregistry)
  AC_CONFIG_SUBDIRS(sipXvxml)
  AC_CONFIG_SUBDIRS(sipXconfig)
  AC_CONFIG_SUBDIRS(sipXpbx)
fi

AC_ARG_ENABLE(conference, 
  AC_HELP_STRING([--enable-conference], 
    [experimental conference server, last known not to compile]),
      conference=yes)

if test x$conference = xyes
then
  AC_CONFIG_SUBDIRS(resiprocateLib)
  AC_CONFIG_SUBDIRS(bbridge)
fi

AC_ARG_ENABLE(phone, 
  AC_HELP_STRING([--enable-phone], 
    [build sipXphone, project is obsolete, see ezphone in sipXcallLib]), 
    phone=yes)

if test x$phone = xyes
then
  AC_CONFIG_SUBDIRS(sipXphone)
fi

SFAC_DIST_DIR
   
AC_CONFIG_FILES([Makefile])

AC_OUTPUT