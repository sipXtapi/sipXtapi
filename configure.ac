#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# benign edit

AC_PREREQ(2.57)
AC_INIT(sipXconfig, 3.1.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])

AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

AC_CONFIG_SRCDIR([neoconf/src/org/sipfoundry/sipxconfig/common/User.java])

SFAC_INIT_FLAGS

CHECK_SSL
SFAC_INIT_FLAGS

# Jar files to all of sipXconfig. 
AC_SUBST(SIPXCONFIG_LIBDIR, [${SIPX_DATADIR}/lib])

AC_ARG_VAR(SIPFOUNDRY_ORG, [If you plan to publish manual to www.sipfoundry.org, set this to where you keep your local copy of http://scm.sipfoundry.org/rep/sipfoundry.org])

# Expand build dir into absolute path
AC_SUBST(BUILDDIR_EXPAND, [`cd $builddir && pwd`])

# turn off the doxygen support shared from other sipX projects
AM_CONDITIONAL(DOC,false)

CHECK_JDK
CHECK_ANT
CHECK_CLOVER

AC_CONFIG_FILES([
    Makefile
    config/sipXcommon.mak
    bin/Makefile
    neoconf/Makefile
    web/Makefile
    doc/Makefile
])

AC_OUTPUT

# autoconf variables like datadir, localstatedir, etc are not resolved
# until make time, so here we get a jump start on this mainly for ANT
# purist that don't do make and got right int ant
make build.properties

