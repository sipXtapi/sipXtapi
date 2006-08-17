#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)
AC_INIT(mod_cplusplus, 1.5.2, sipx-dev@sipfoundry.org)

AC_CONFIG_AUX_DIR([config])
m4_include([config/general.m4])
m4_include([config/sipXlib.m4])

AM_INIT_AUTOMAKE(foreign)

CHECK_APACHE2

SFAC_INIT_FLAGS
SFAC_DOWNLOAD_DEPENDENCIES

AC_CONFIG_FILES([
  Makefile
])

AC_OUTPUT
