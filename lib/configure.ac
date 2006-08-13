AC_PREREQ(2.57)
AC_INIT(sipXlib, 0.0.0, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE(foreign)

# default, fc5
cgicc=yes
mod_cplusplus=yes
xerces_c=yes

AC_ARG_ENABLE(fc4,
	AC_HELP_STRING([--enable-fc4], [Enable dependencies for Fedora Core 4]),
	[ 
          java_sun=yes
          ruby_gems=yes
        ])

AC_ARG_ENABLE(fc5,
	AC_HELP_STRING([--enable-fc5], [Enable dependencies for Fedora Core 5]),
	[ 
          java_sun=yes
          ruby_gems=yes
        ])

AC_ARG_ENABLE(rhe4,
	AC_HELP_STRING([--enable-rhe4], [Enable dependencies for RHE4 and Centos4]),
	[ 
          java_sun=yes
          ruby_gems=yes
          pcre=yes
          ruby=yes
          w3c_libwww=yes
        ])

AC_ARG_ENABLE(cgicc,
	AC_HELP_STRING([--enable-cgicc], [Build cgicc dependency]),
	[ cgicc=${enableval} ])
test x$cgicc = xyes && AC_CONFIG_SUBDIRS(cgicc)

AC_ARG_ENABLE(cppunit,
	AC_HELP_STRING([--enable-cppunit], [Build cppunit dependency]),
	[ cppunit=${enableval} ])
test x$cppunit = xyes && AC_CONFIG_SUBDIRS(cppunit)

AC_ARG_ENABLE(java-ibm,
	AC_HELP_STRING([--enable-java-ibm], [Build IBM Java SDK dependency]),
	[ java_ibm=${enableval} ])
test x$java_ibm = xyes && AC_CONFIG_SUBDIRS(java-ibm)

AC_ARG_ENABLE(java-sun,
	AC_HELP_STRING([--enable-java-sun], [Build Sun Java SDK dependency]),
	[ java_sun=${enableval} ])
test x$java_sun = xyes && AC_CONFIG_SUBDIRS(java-sun)

AC_ARG_ENABLE(mod_cplusplus,
	AC_HELP_STRING([--enable-mod_cplusplus], [Build mod_cplusplus dependency]),
	[ mod_cplusplus=${enableval} ])
test x$mod_cplusplus = xyes && AC_CONFIG_SUBDIRS(mod_cplusplus)

AC_ARG_ENABLE(pcre,
	AC_HELP_STRING([--enable-pcre], [Build pcre dependency]),
	[ pcre=${enableval} ])
test x$pcre = xyes && AC_CONFIG_SUBDIRS(pcre)

AC_ARG_ENABLE(ruby,
	AC_HELP_STRING([--enable-ruby], [Build ruby dependency]),
	[ ruby=${enableval} ])
test x$ruby = xyes && AC_CONFIG_SUBDIRS(ruby)

AC_ARG_ENABLE(ruby-gems,
	AC_HELP_STRING([--enable-ruby-gems], [Build ruby gems dependency]),
	[ ruby_gems=${enableval} ])
test x$ruby_gems = xyes && AC_CONFIG_SUBDIRS(ruby-gems)

AC_ARG_ENABLE(ruby-postgres,
	AC_HELP_STRING([--enable-ruby-postgres], [Build ruby postgres dependency]),
	[ ruby_postgres=${enableval} ])
test x$ruby_postgres = xyes && AC_CONFIG_SUBDIRS(ruby-postgres)

AC_ARG_ENABLE(ruby-rails,
	AC_HELP_STRING([--enable-ruby-rails], [Build ruby rails dependency]),
	[ ruby_rails=${enableval} ])
test x$ruby_rails = xyes && AC_CONFIG_SUBDIRS(ruby-rails)

AC_ARG_ENABLE(w3c-libwww,
	AC_HELP_STRING([--enable-w3c-libwww], [Build w3c-libwww w/ssl dependency]),
	[ w3c_libwww=${enableval} ])
test x$w3c_libwww = xyes && AC_CONFIG_SUBDIRS(w3c-libwww)

AC_ARG_ENABLE(xerces-c,
	AC_HELP_STRING([--enable-xerces-c], [Build xerces-c dependency]),
	[ xerces_c=${enableval} ])
test x$xerces_c = xyes && AC_CONFIG_SUBDIRS(xerces-c)

SFAC_RPM_REPO
   
AC_CONFIG_FILES([Makefile])

AC_OUTPUT

