##
## AC macros for general packages like OpenSSL, Xerces, etc
##

# ============= A U T O C O N F ===============
AC_DEFUN([CHECK_AUTOCONF],
[
    AC_PATH_PROG([AUTOCONF], autoconf)
    if test `autoconf --version | grep "2.58" | wc -c` -ne 0 ; then
        AC_MSG_WARN(["Autoconf 2.58 was found on system.  If you are a maintainer of this library it has known incompatilities.  If you are not a maintainer, 2.58 has serious bugs and you should consider upgrading autoconf"]);
    fi
])


# ============ C L O V E R  =======================
AC_DEFUN([CHECK_CLOVER],
[
   AC_ARG_VAR(CLOVER_JAR, [Clover home directory])

   if test x_"${CLOVER_JAR}" != x_
   then
       AC_CHECK_FILE([$CLOVER_JAR],
       [
           CLOVER_JAR=$CLOVER_RPM_JAR
       ],
       [
           AC_MSG_ERROR([Invalid CLOVER_JAR environment variable: Cannot find $CLOVER_JAR.])
       ])
   else
       CLOVER_RPM_JAR=/usr/share/java/ant/clover.jar
       AC_CHECK_FILE([$CLOVER_RPM_JAR],
       [
           CLOVER_JAR=$CLOVER_RPM_JAR
       ],)
   fi
])

# ============= C P P U N I T ==================
dnl
dnl AM_PATH_CPPUNIT(MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
AC_DEFUN([AM_PATH_CPPUNIT],
[

AC_ARG_WITH(cppunit-prefix,[  --with-cppunit-prefix=PFX   Prefix where CppUnit is installed (optional)],
            cppunit_config_prefix="$withval", cppunit_config_prefix="")
AC_ARG_WITH(cppunit-exec-prefix,[  --with-cppunit-exec-prefix=PFX  Exec prefix where CppUnit is installed (optional)],
            cppunit_config_exec_prefix="$withval", cppunit_config_exec_prefix="")

  if test x$cppunit_config_exec_prefix != x ; then
     cppunit_config_args="$cppunit_config_args --exec-prefix=$cppunit_config_exec_prefix"
     if test x${CPPUNIT_CONFIG+set} != xset ; then
        CPPUNIT_CONFIG=$cppunit_config_exec_prefix/bin/cppunit-config
     fi
  fi
  if test x$cppunit_config_prefix != x ; then
     cppunit_config_args="$cppunit_config_args --prefix=$cppunit_config_prefix"
     if test x${CPPUNIT_CONFIG+set} != xset ; then
        CPPUNIT_CONFIG=$cppunit_config_prefix/bin/cppunit-config
     fi
  fi          

  AC_PATH_PROG(CPPUNIT_CONFIG, cppunit-config, no)
  cppunit_version_min=$1

  AC_MSG_CHECKING(for Cppunit - version >= $cppunit_version_min)
  no_cppunit=""
  if test "$CPPUNIT_CONFIG" = "no" ; then
    AC_MSG_RESULT(no)
    no_cppunit=yes
  else
    CPPUNIT_CFLAGS=`$CPPUNIT_CONFIG --cflags`
    CPPUNIT_LIBS=`$CPPUNIT_CONFIG --libs`
    cppunit_version=`$CPPUNIT_CONFIG --version`

    cppunit_major_version=`echo $cppunit_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    cppunit_minor_version=`echo $cppunit_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    cppunit_micro_version=`echo $cppunit_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    cppunit_major_min=`echo $cppunit_version_min | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    if test "x${cppunit_major_min}" = "x" ; then
       cppunit_major_min=0
    fi

    cppunit_minor_min=`echo $cppunit_version_min | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    if test "x${cppunit_minor_min}" = "x" ; then
       cppunit_minor_min=0
    fi

    cppunit_micro_min=`echo $cppunit_version_min | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x${cppunit_micro_min}" = "x" ; then
       cppunit_micro_min=0
    fi                                                                                                                  

    cppunit_version_proper=`expr \
        $cppunit_major_version \> $cppunit_major_min \| \
        $cppunit_major_version \= $cppunit_major_min \& \
        $cppunit_minor_version \> $cppunit_minor_min \| \
        $cppunit_major_version \= $cppunit_major_min \& \
        $cppunit_minor_version \= $cppunit_minor_min \& \
        $cppunit_micro_version \>= $cppunit_micro_min `

    if test "$cppunit_version_proper" = "1" ; then
      AC_MSG_RESULT([$cppunit_major_version.$cppunit_minor_version.$cppunit_micro_version])
    else
      AC_MSG_RESULT(no)
      no_cppunit=yes
    fi
  fi

  if test "x$no_cppunit" = x ; then
     ifelse([$2], , :, [$2])
  else
     CPPUNIT_CFLAGS=""
     CPPUNIT_LIBS=""
     ifelse([$3], , :, [$3])
  fi

  AC_SUBST(CPPUNIT_CFLAGS)
  AC_SUBST(CPPUNIT_LIBS)
])   

AC_DEFUN([CHECK_CPPUNIT],
[
    AM_PATH_CPPUNIT(1.9,
      [],
      [AC_MSG_ERROR("cppunit not found")]
    )
])
                                                                                                                        

# ============ T O M C A T  =======================
AC_DEFUN([CHECK_TOMCAT_HOME],
[
   AC_ARG_VAR(TOMCAT_HOME, [Apache Tomcat home directory])
   for dir in $TOMCAT_HOME $CATALINA_HOME /usr/local/tomcat; do
       # simply test for directory
       if test -d $dir; then
           TOMCAT_HOME=$dir;
           found_tomcat="yes";

           # older JVM's working with older tomcats need servlet.jar from tomcat
           # attempt to find it, by don't complain if you can't. Should break this
           # into separate macro CHECK_TOMCAT5
           jar_path=$dir/lib:$dir/common/lib;
           AC_PATH_PROG(SERVLET_JAR, servlet.jar, ,$jar_path)
           if test x$SERVLET_JAR != x; then
               AC_SUBST(SERVLET_JAR)
               break;
           fi
       fi
   done

   if test x_$found_tomcat != x_yes; then
       AC_MSG_ERROR([Cannot tomcat. You may try setting the TOMCAT_HOME or CATALINA_HOME environment \
           variable if you haven't already done so])
   fi

])


# ============ J D K  =======================
AC_DEFUN([CHECK_JDK],
[
    AC_ARG_VAR(JAVA_HOME, [Java Development Kit])

    TRY_JAVA_HOME=`ls -dr /usr/java/* 2> /dev/null | head -n 1`
    for dir in $JAVA_HOME $JDK_HOME /usr/local/jdk /usr/local/java $TRY_JAVA_HOME; do
        AC_PATH_PROG(jar, dt.jar, ,$dir/lib)
        if test x$jar != x; then
            found_jdk="yes";
            JAVA_HOME=$dir
            break;
        fi
    done

    if test x_$found_jdk != x_yes; then
        AC_MSG_ERROR([Cannot find dt.jar in expected location. You may try setting the JAVA_HOME environment variable if you haven't already done so])
    fi

    AC_SUBST(JAVA, [$JAVA_HOME/jre/bin/java])

    AC_ARG_VAR(JAVAC_OPTIMIZED, [Java compiler option for faster performance. Default is on])
    test -z $JAVAC_OPTIMIZED && JAVAC_OPTIMIZED=on

    AC_ARG_VAR(JAVAC_DEBUG, [Java compiler option to reduce code size. Default is off])
    test -z $JAVAC_DEBUG && JAVAC_DEBUG=off
])


# ============ J N I =======================
AC_DEFUN([CHECK_JNI],
[
   CHECK_JDK

   JAVA_HOME_INCL=$JAVA_HOME/include
   AC_CHECK_FILE([$JAVA_HOME_INCL/jni.h],
       [
           XFLAGS="-I$JAVA_HOME_INCL -I$JAVA_HOME_INCL/linux";
           CFLAGS="$XFLAGS $CFLAGS";
           CXXFLAGS="$XFLAGS $CXXFLAGS";

           ## i386 is a big assumption, TODO: make smarter
           JAVA_LIB_DIR="$JAVA_HOME/jre/lib/i386";

           ## Effectively LD_LIBRARY_PATH for JVM for unittests or anything else
           AC_SUBST(JAVA_LIB_PATH, [$JAVA_LIB_DIR:$JAVA_LIB_DIR/client])

           LDFLAGS="$LDFLAGS -L$JAVA_LIB_DIR -ljava -lverify"

           ## Use client flags as only call for this is phone. config server
           ## should use jre/lib/i386/server, but not a big deal
           LDFLAGS="$LDFLAGS -L$JAVA_LIB_DIR/client -ljvm"
       ],
       AC_MSG_ERROR([Cannot find or validate header file $JAVA_HOME_INCL/jni.h]))
])


# ============ A N T  ==================
AC_DEFUN([CHECK_ANT],
[
   AC_REQUIRE([AC_EXEEXT])
   AC_ARG_VAR(ANT, [Ant program])

   test -z $ANT_HOME || ANT_HOME_BIN=$ANT_HOME/bin
   for dir in $ANT_HOME_BIN $PATH /usr/local/ant/bin; do
       # only works because unix does not use ant.sh
       AC_PATH_PROG(ANT, ant$EXEEXT ant.bat, ,$dir)
       if test x$ANT != x; then
           found_ant="yes";
           break;
       fi
   done

   if test x_$found_ant != x_yes; then
       AC_MSG_ERROR([Cannot find ant program. Try setting ANT_HOME environment variable or use 'configure ANT=<path to ant executable>])
   fi
])


# ============ O P E N S S L ==================
#
# OpenSSL is required
#
AC_DEFUN([CHECK_SSL],
[   AC_ARG_WITH(openssl,
                [--with-openssl=PATH to openssl source directory],
                [openssl_path=$withval],
                [openssl_path="/usr/local /usr/local/ssl /usr/ssl /usr/pkg /usr / /sw/lib"]
                )
    AC_PATH_PROG([OPENSSL],[openssl])
    AC_MSG_CHECKING([for openssl includes])
    found_ssl_inc="no";
    tried_path=""
    for dir in $openssl_path ; do
        if test -f "$dir/openssl/ssl.h"; then
            found_ssl_inc="yes";
            sslincdir="$dir"
            break;
        elif test -f "$dir/include/openssl/ssl.h"; then
            found_ssl_inc="yes";
            sslincdir="$dir/include"
            break;
        else
            tried_path="${tried_path} $dir $dir/include"
        fi
    done
    if test x_$found_ssl_inc != x_yes ; then
        AC_MSG_ERROR(['openssl/ssl.h' not found; tried ${tried_path}])
    else
        AC_MSG_RESULT($sslincdir)
        HAVE_SSL=yes
        AC_SUBST(HAVE_SSL)
        SSL_CFLAGS="-DHAVE_SSL"

        # don't need to add -I/usr/include
        if test "${sslincdir}" != "/usr/include"; then
           SSL_CFLAGS="$SSL_CFLAGS -I$sslincdir"
        fi
    fi

    AC_MSG_CHECKING([for openssl libraries])
    found_ssl_lib="no";
    for dir in $openssl_path ; do
        if test -f "$dir/lib/libssl.so" -o "$dir/lib/libssl.a"; then
            found_ssl_lib="yes";
            ssllibdir="$dir/lib"
            break;
        # This test is an ugly hack to make sure that the current builds work.
        # But our test should be improved to allow libssl.so to have any version
        # and let the test succeed, since "-lssl" works with any version number.
        elif test -f "$dir/lib/libssl.so.4"; then
            found_ssl_lib="yes";
            ssllibdir="$dir/lib"
            break;
        elif test -f "$dir/lib/openssl/libssl.so"; then
            found_ssl_lib="yes";
            ssllibdir="$dir/lib/openssl"
            break;
        elif test -f "$dir/lib/ssl/libssl.so"; then
            found_ssl_lib="yes";
            ssllibdir="$dir/lib/ssl"
            break;
        fi
    done

    if test x_$found_ssl_lib != x_yes ; then
        AC_MSG_ERROR(['libssl.so' not found; tried $openssl_path, each with lib, lib/openssl, and lib/ssl])
    else
        AC_MSG_RESULT($ssllibdir)
        AC_SUBST(SSL_LDFLAGS,"-L$ssllibdir")
        AC_SUBST(SSL_LIBS,"-lssl -lcrypto")
    fi

## openssl-devel rpm installs kerbose in another dir
    AC_MSG_CHECKING(for extra kerberos includes)
    krb_found="no"
    for krbdir in $openssl_path ; do
      if test -f "$krbdir/kerberos/include/krb5.h"; then
        krb_found="yes"
        break;
      fi
    done
    if test x_$krb_found = x_yes; then
        AC_MSG_RESULT($krbdir/kerberos/include)
        SSL_CFLAGS="$SSL_CFLAGS -I$krbdir/kerberos/include"
    else
        AC_MSG_RESULT(['kerberos/include/krb5.h' not found - looked in $openssl_path])
    fi

    AC_SUBST(SSL_CFLAGS,"$SSL_CFLAGS")
    AC_SUBST(SSL_CXXFLAGS,"$SSL_CFLAGS")
])


# ============ L I B R T  =========================
AC_DEFUN([CHECK_LIBRT],
[
   AC_MSG_CHECKING([for librt])

   rt_found="no"
   for dir in /lib /usr/lib /usr/local/lib; do
      if test -f "$dir/librt.so.1"; then
        rt_found="yes"
        break;
      fi
   done
   if test x_$rt_found = x_yes; then
        AC_SUBST(RT_LIBS,"-lrt")
	AC_MSG_RESULT([-lrt])
   else
        AC_SUBST(RT_LIBS,"")
        AC_MSG_RESULT([not needed])
   fi
])

# ============ C O R E A U D I O =======================

AC_DEFUN([CHECK_COREAUDIO],
[
    AC_MSG_CHECKING([for CoreAudio])

    if test "`uname`" == "Darwin"; then
        AC_SUBST(CA_LIBS,"-framework CoreAudio -framework AudioToolbox")
        AC_MSG_RESULT([yes])
    else
        AC_SUBST(CA_LIBS,"")
        AC_MSG_RESULT([not needed])
    fi
])

# ============ C A R B O N ==================

AC_DEFUN([CHECK_CARBON],
[
    AC_MSG_CHECKING([for Carbon])

    if test "`uname`" == "Darwin"; then
        AC_SUBST(CARBON_LIBS,"-framework Carbon")
        AC_MSG_RESULT([yes])
    else
        AC_SUBST(CARBON_LIBS,"")
        AC_MSG_RESULT([not needed])
    fi
])


# ============ X E R C E S ==================
AC_DEFUN([CHECK_XERCES],
[   AC_MSG_CHECKING([for xerces])
    AC_ARG_WITH(xerces,
                [--with-xerces=PATH to xerces source directory],
                [xerces_path=$with_val],
                [xerces_path="/usr/local/xercesc /usr/lib/xercesc /usr/xercesc /usr/pkg /usr/local /usr"]
                )
    for dir in $xerces_path ; do
        xercesdir="$dir"
        if test -f "$dir/include/xercesc/sax/Parser.hpp"; then
            found_xerces="yes";
            XERCES_CFLAGS="-I$xercesdir/include/xercesc";
            break;
        fi
        if test -f "$dir/include/sax/Parser.hpp"; then
            found_xerces="yes";
            XERCES_CFLAGS="-I$xercesdir/include";
            break;
        fi
    done

    if test x_$found_xerces != x_yes; then
        AC_MSG_ERROR(Cannot find xerces - looked for include/sax/Parser.hpp or include/xercesc/sax/Parser.hpp in $xerces_path )
    else
        AC_MSG_RESULT($xercesdirm)

        AC_SUBST(XERCES_CFLAGS,"$XERCES_CFLAGS")
        AC_SUBST(XERCES_CXXFLAGS,"$XERCES_CFLAGS")

        AC_SUBST(XERCES_LIBS,["-lxerces-c"])
        AC_SUBST(XERCES_LDFLAGS,["-L$xercesdir/lib"])
    fi
],
[
    AC_MSG_RESULT(yes)
])
])dnl

# CHECK_APR is called from CHECK_APACHE2
# ============ A P R ==============
AC_DEFUN([CHECK_APR],
[
    found_apr_dir="no"
    AC_MSG_CHECKING([for apr headers])
    # May need to add support for Apache post-2.0.50 tarball
    AC_ARG_WITH(apr,
                [--with-apr=PATH to apr header files directory],
                [apr_path=$withval],
                [apr_path="/usr/include/httpd /usr/include/apr-0 /usr/local/apache2/include /usr/apache2/include /etc/httpd/include /usr/include/apache2"
                ]
               )
    for apr_dir in $apr_path ; do
       if test -f "$apr_dir/apr.h"; then
          found_apr_dir="yes"
          break;
       fi
    done

    if test x_$found_apr_dir != x_yes; then
       AC_MSG_ERROR(['apr.h' not found; tried $apr_path])
    else
       AC_MSG_RESULT($apr_dir)
       APACHE2_CFLAGS="$APACHE2_CFLAGS -I$apr_dir"
       APACHE2_CXXFLAGS="$APACHE2_CXXFLAGS -I$apr_dir"
    fi
])dnl

# ============ A P A C H E 2 ==================
AC_DEFUN([CHECK_APACHE2],
[
   found_apache2_inc="no"
   AC_ARG_WITH([apache-include],
               [--with-apache-include=PATH the apache2 include directory],
               [ apache2_inc_search_path=$withval
                 ],
               [ apache2_inc_search_path="/usr/local/apache2/include /usr/apache2/include /etc/httpd/include /usr/include/apache2 /usr/include/httpd"
                 ]
               )

   ## Include directory
   AC_MSG_CHECKING([for apache2 include directory])
   for incdir in $apache2_inc_search_path; do
          if test -f "$incdir/httpd.h"; then
       found_apache2_inc="yes";
       break;
     fi
   done
   if test x_$found_apache2_inc = x_yes; then
       AC_MSG_RESULT($incdir)
       AC_SUBST(APACHE2_CFLAGS, -I$incdir)
       AC_SUBST(APACHE2_CXXFLAGS, -I$incdir)
       AC_SUBST(APACHE2_INCDIR, $incdir)
   else
       AC_MSG_ERROR('httpd.h' not found; tried: $apache2_inc_search_path)
   fi

   CHECK_APR

   AC_ARG_WITH([apache-modules],
               [--with-apache-modules=PATH where apache modules are installed],
               [ apache2_mod_search_path="$withval"
                 apache2_mod_override="$withval"
                ],
               [ apache2_mod_search_path="/usr/local/apache2/modules /usr/apache2/modules /etc/httpd/modules /usr/lib/httpd/modules /usr/lib/apache2 /usr/lib/apache2/modules"
                 apache2_mod_override=""
                ]
              )

   ## Get the version numbers for this Apache installation.
   ## APACHE2_MMN is the module magic number, which is the version of
   ## the API that modules have to interface to.
   ## Some versions have a $incdir/.mmn file containing only the MMN, but
   ## we can't depend on that.
   apache2_mmn=`sed <$incdir/ap_mmn.h \
                -e '/#define MODULE_MAGIC_NUMBER_MAJOR/!d' \
                -e 's/#define MODULE_MAGIC_NUMBER_MAJOR //'`
   AC_SUBST(APACHE2_MMN, $apache2_mmn)
   AC_MSG_RESULT(apachd2_mmn=$apache2_mmn)
   ## APACHE2_VERSION is the Apache version number.
   ## This makes it easier for the uninitiated to see what versions of Apache
   ## might be compatible with this mod_cplusplus.  But compatibility is really
   ## controlled by the MMN value.
   apache2_version=`awk -f $srcdir/config/apache_version.awk $incdir/ap_release.h`
   AC_SUBST(APACHE2_VERSION, $apache2_version)
   AC_MSG_RESULT(apache2_version=$apache2_version)
   AC_MSG_RESULT(apache2_version=$apache2_version)

   ## Apache Modules Directory
   AC_MSG_CHECKING([for apache2 modules directory])
   found_apache2_mod="no";
   tried_path=""
   ## Older versions of Apache seem to always have mod_access.so in their
   ## modules directory.  Newer ones can have it linked into the httpd
   ## executable, but they seem to have an httpd.exp file in the modules
   ## directory.  So we check for either.
   for apache2_moddir in $apache2_mod_search_path; do
     if test -f "$apache2_moddir/mod_access.so"; then
       found_apache2_mod="yes";
       break;
     elif test -f "$apache2_moddir/httpd.exp"; then
       found_apache2_mod="yes";
       break;
     else
       tried_path="${tried_path} $apache2_moddir"
     fi
   done
   if test x_$found_apache2_mod = x_yes; then
       AC_MSG_RESULT($apache2_moddir)
       AC_SUBST(APACHE2_MOD, $apache2_moddir)
   elif test x_$apache2_mod_override != x_; then
       AC_SUBST(APACHE2_MOD, $apache2_mod_override)
       AC_MSG_WARN('mod_access.so' and 'httpd.exp' not found; using explicit value: $apache2_mod_override)
   else
       AC_MSG_ERROR('mod_access.so' and 'httpd.exp' not found; tried: $tried_path)
   fi

   AC_ARG_WITH([apache-httpd],
               [--with-apache-httpd=PATH the apache2 httpd executable],
               [ apache2_bin_search_path="$withval"
                 ],
               [ apache2_bin_search_path="/usr/local/apache2/bin /usr/apache2/bin /etc/httpd/bin /usr/local/sbin /usr/local/bin /usr/sbin /usr/bin /usr/sbin/apache2"
                 ]
               )

   ## Apache httpd executable
   AC_MSG_CHECKING([for Apache2 httpd])
   found_apache2_httpd="no";
   for apache2_httpd_dir in $apache2_bin_search_path; do
     if test -x "$apache2_httpd_dir/httpd"; then
       found_apache2_httpd="yes";
       apache2_httpd="$apache2_httpd_dir/httpd";
       break;
     elif test -x "$apache2_httpd_dir/httpd2"; then
       found_apache2_httpd="yes";
       apache2_httpd="$apache2_httpd_dir/httpd2";
       break;
     elif test -f "$apache2_httpd_dir" -a -x "$apache2_httpd_dir"; then
       found_apache2_httpd="yes";
       apache2_httpd="$apache2_httpd_dir";
       break;
     fi
   done
   if test x_$found_apache2_httpd = x_yes; then
       AC_MSG_RESULT($apache2_httpd)
       AC_SUBST(APACHE2_HTTPD, $apache2_httpd)
   else
       AC_MSG_ERROR('httpd' not found; tried: $apache2_bin_search_path)
   fi

   ## Apache apxs executable
   AC_ARG_WITH([apache-apxs],
               [--with-apache-apxs=PATH the apache2 apxs executable],
               [ apache2_apxs_search_path="$withval"
                 ],
               [ apache2_apxs_search_path="/usr/local/apache2/bin /usr/apache2/bin /etc/httpd/bin /usr/local/sbin /usr/local/bin /usr/sbin /usr/bin"
                 ]
               )
   AC_MSG_CHECKING([for apache2 apxs])
   found_apache2_apxs="no";
   for apache2_apxs_dir in $apache2_apxs_search_path; do

     ## Apache >= 2.0.50
     if test -x "$apache2_apxs_dir/apxs2"; then
       found_apache2_apxs="yes";
       apache2_apxs="$apache2_apxs_dir/apxs2";
       break;

     ## Apache < 2.0.50
     elif test -x "$apache2_apxs_dir/apxs"; then
       found_apache2_apxs="yes";
       apache2_apxs="$apache2_apxs_dir/apxs";
       break;
     fi
   done
   if test x_$found_apache2_apxs = x_yes; then
       AC_MSG_RESULT($apache2_apxs)
       AC_SUBST(APACHE2_APXS, $apache2_apxs)
   else
       AC_MSG_ERROR('apxs' not found; tried: $apache2_apxs_search_path)
   fi

   ## Apache "home", the location of build/config_vars.mk, and probably
   ## the parent of APACHE2_INC, etc.
   AC_ARG_WITH([apache-home],
               [--with-apache-home=PATH the apache2 home directory],
               [ apache2_home_search_path="$withval"
                 ],
               [ apache2_home_search_path="/usr/local/apache2 /usr/apache2 /etc/httpd /usr/local/sbin /usr/local /usr/sbin /usr /usr/lib/apache2 /usr/share/apache2"
                 ]
               )
   AC_MSG_CHECKING([for apache2 home])
   found_apache2_home="no";
   for apache2_home_dir in $apache2_home_search_path; do
   if test -f "$apache2_home_dir/build/config_vars.mk"; then
       found_apache2_home="yes";
       apache2_home="$apache2_home_dir";
       break;
     fi
   done
   if test x_$found_apache2_home = x_yes; then
       AC_MSG_RESULT($apache2_home)
       AC_SUBST(APACHE2_HOME, $apache2_home)
   else
       AC_MSG_ERROR('build/config_vars.mk' not found; tried: $apache2_home_search_path)
   fi
])dnl

# ============ M O D   C P L U S P L U S ==================
AC_DEFUN([CHECK_MODCPLUSPLUS],
[
    AC_MSG_CHECKING([for mod_cplusplus])
    AC_ARG_WITH(mod_cplusplus,
                [--with-mod_cplusplus=PATH to mod_cplusplus source directory],
                [mod_cplusplus_path=$withval],
                [mod_cplusplus_path="/usr/local/apache2/include /usr/local/include /usr/include /usr/include/httpd /usr/include/apache2"],
                )
    for mod_cplusplusdir in $mod_cplusplus_path ; do
            if test -f "$mod_cplusplusdir/mod_cplusplus.h";
        then
            found_mod_cplusplus="yes";
            break;
        fi
    done

    if test x_$found_mod_cplusplus != x_yes;
    then
        AC_MSG_ERROR(['mod_cplusplus.h' not found; tried $mod_cplusplus_path])
    else
        AC_MSG_RESULT($mod_cplusplusdir)
    fi
])dnl

# ==================== C G I C C  =========================
AC_DEFUN([CHECK_CGICC],
[
    AC_MSG_CHECKING([for cgicc])
    AC_ARG_WITH(cgicc,
                [--with-cgicc=PATH to cgicc library],
                [cgicc_path=$withval],
                [
                 AC_PATH_PROG([CGICC_CONFIG],cgicc-config)
                 if test "x_$CGICC_CONFIG" != "x_"; then
                   cgicc_path=`$CGICC_CONFIG --prefix`
                 else
                   cgicc_path="/usr/local/cgicc /usr/local /usr"
                 fi
                ]
                )

    found_cgicc="no";
    for cgicc_dir in $cgicc_path; do
           if test -f "$cgicc_dir/lib/libcgicc.so" -a -f "$cgicc_dir/include/cgicc/Cgicc.h"
       then
         found_cgicc="yes";
         break;
      fi
    done

    if test x_$found_cgicc = x_yes
    then
        AC_MSG_RESULT($cgicc_dir)

        AC_SUBST(CGICC_CFLAGS,"-I$cgicc_dir/include")
        AC_SUBST(CGICC_LIBS,"$cgicc_dir/lib/libcgicc.la")
    else
        AC_MSG_ERROR([cgicc not found - looked for lib/libcgicc.so and include/cgicc/Cgicc.h in $cgicc_path])
    fi
])dnl


# ============ L I B W W W ==================
AC_DEFUN([CHECK_LIBWWW],
[   AC_MSG_CHECKING([for libwww])
    AC_ARG_WITH(libwww,
                [--with-libwww=PATH to libwww source directory],
                )
    for dir in $withval /usr/local/w3c-libwww /usr/lib/w3c-libwww /usr/w3c-libwww /usr/pkg /usr/local /usr; do
        lwwwdir="$dir"
        if test -f "$dir/include/w3c-libwww/WWWLib.h"; then
            found_www="yes";
            LIBWWW_CFLAGS="-I$lwwwdir/include/w3c-libwww";
            LIBWWW_CXXFLAGS="-I$lwwwdir/include/w3c-libwww";
            break;
        fi
        if test -f "$dir/include/WWWLib.h"; then
            found_www="yes";
            LIBWWW_CFLAGS="-I$lwwwdir/include";
            LIBWWW_CXXFLAGS="-I$lwwwdir/include ";
            break;
        fi
    done

    if test x_$found_www != x_yes; then
        AC_MSG_ERROR(not found; 'include/w3c-libwww/WWWLib.h' and 'include/WWWLib.h' not in any of: $withval /usr/local/w3c-libwww /usr/lib/w3c-libwww /usr/w3c-libwww /usr/pkg /usr/local /usr)
    fi
    if test ! -e "$dir/lib/libwwwapp.so";then
        AC_MSG_ERROR(not found; 'libwwwapps.so' not in: $dir/lib)
    fi
    if test ! -e "$dir/lib/libwwwssl.so";then
        AC_MSG_ERROR(not found; 'libwwwssl.so' not in: $dir/lib)
    fi
    if test x_$found_www = x_yes; then
        AC_MSG_RESULT($lwwwdir)

        AC_SUBST(LIBWWW_CFLAGS)
        AC_SUBST(LIBWWW_CXXFLAGS)

        LIBWWW_LIBS="-lwwwapp -lwwwfile -lwwwhttp -lwwwssl -lwwwcore";
        LIBWWW_LIBS="$LIBWWW_LIBS -lwwwinit -lwwwapp -lwwwhttp -lwwwcache -lwwwcore";
        LIBWWW_LIBS="$LIBWWW_LIBS -lwwwfile -lwwwutils -lwwwmime -lwwwstream -lmd5";
        LIBWWW_LIBS="$LIBWWW_LIBS -lpics -lwwwnews -lwwwdir -lwwwtelnet -lwwwftp";
        LIBWWW_LIBS="$LIBWWW_LIBS -lwwwmux -lwwwhtml -lwwwgopher -lwwwtrans -lwwwzip";
        LIBWWW_LIBS="$LIBWWW_LIBS -lwwwssl -lwwwxml -lxmlparse -lxmltok";
        AC_SUBST(LIBWWW_LIBS)

        LIBWWW_LDFLAGS="-L$lwwwdir/lib";
        AC_SUBST(LIBWWW_LDFLAGS)
    fi
],
[
    AC_MSG_RESULT(yes)
])
])dnl

# ============ P C R E ==================
AC_DEFUN([CHECK_PCRE],
[   AC_MSG_CHECKING([for pcre >= 4.5])
    # Process the --with-pcre argument which gives the pcre base directory.
    AC_ARG_WITH(pcre,
                [--with-pcre=PATH path to pcre install directory],
                )
    homeval=$withval
    # Have to unset withval so we can tell if --with-pcre_includedir was
    # specified, as AC_ARG_WITH will not unset withval if the option is not
    # there!
    withval=

    # Process the --with-pcre_includedir argument which gives the pcre include
    # directory.
    AC_ARG_WITH(pcre_includedir,
                [--with-pcre_includedir=PATH path to pcre include directory (containing pcre.h)],
                )
    # If withval is set, use that.  If not and homeval is set, use
    # $homeval/include.  If neither, use null.
    includeval=${withval:-${homeval:+$homeval/include}}
    withval=

    # Process the --with-pcre_libdir argument which gives the pcre library
    # directory.
    AC_ARG_WITH(pcre_libdir,
                [--with-pcre_libdir=PATH path to pcre lib directory (containing libpcre.{so,a})],
                )
    libval=${withval:-${homeval:+$homeval/lib}}

    # Check for pcre.h in the specified include directory if any, and a number
    # of other likely places.
    for dir in $includeval /usr/local/include /usr/local/pcre/include /usr/include /usr/include/pcre /sw/include; do
        if test -f "$dir/pcre.h"; then
            found_pcre_include="yes";
            includeval=$dir
            break;
        fi
    done

    # Check for libpcre.{so,a} in the specified lib directory if any, and a
    # number of other likely places.
    for dir in $libval /usr/local/lib /usr/local/pcre/lib /usr/lib /sw/lib; do
        if test -f "$dir/libpcre.so" -o -f "$dir/libpcre.a"; then
            found_pcre_lib="yes";
            libval=$dir
            break;
        fi
    done

    # Test that we've been able to find both directories, and set the various
    # makefile variables.
    if test x_$found_pcre_include != x_yes; then
        AC_MSG_ERROR(Cannot find pcre.h - looked in $includeval)
    else
        if test x_$found_pcre_lib != x_yes; then
            AC_MSG_ERROR(Cannot find libpcre.so or libpcre.a libraries - looked in $libval)
        else
            ## Test for version
            pcre_ver=`pcre-config --version`
            AX_COMPARE_VERSION([$pcre_ver],[ge],[4.2])

            if test "x_$ax_compare_version" = "x_false"; then
               AC_MSG_ERROR(Found pcre version $pcre_ver)
            else
               AC_MSG_RESULT($pcre_ver is ok)
            fi
            AC_MSG_RESULT($includeval and $libval)

            PCRE_CFLAGS="-I$includeval"
            PCRE_CXXFLAGS="-I$includeval"
            AC_SUBST(PCRE_CFLAGS)
            AC_SUBST(PCRE_CXXFLAGS)

            AC_SUBST(PCRE_LIBS, "-lpcre" )
            AC_SUBST(PCRE_LDFLAGS, "-L$libval")
        fi
    fi
])dnl


# ============ D O X Y G E N ==================
# Originaly from CppUnit BB_ENABLE_DOXYGEN

AC_DEFUN(ENABLE_DOXYGEN,
[
AC_ARG_ENABLE(doxygen, [  --enable-doxygen        enable documentation generation with doxygen (yes)], [], [ enable_doxygen=yes])
AC_ARG_ENABLE(dot, [  --enable-dot            use 'dot' to generate graphs in doxygen (auto)])
AC_ARG_ENABLE(html-docs, [  --enable-html-docs      enable HTML generation with doxygen (yes)], [], [ enable_html_docs=yes])
AC_ARG_ENABLE(latex-docs, [  --enable-latex-docs     enable LaTeX documentation generation with doxygen (no)], [], [ enable_latex_docs=no])
if test "x$enable_doxygen" = xno; then
        enable_doc=no
else
        AC_MSG_CHECKING([for doxygen documentation processor])
        AC_PATH_PROG(DOXYGEN, doxygen, , $PATH)
        if test "x$DOXYGEN" = x; then
              AC_MSG_WARN([could not find doxygen - disabled])
              enable_doxygen=no
              enable_doc=no
        else
                enable_doc=yes
                AC_PATH_PROG(DOT, dot, , $PATH)
        fi
fi

if test x$DOT = x; then
        if test "x$enable_dot" = xyes; then
                AC_MSG_ERROR([could not find dot])
        fi
        enable_dot=no
else
        enable_dot=yes
fi
AC_SUBST(enable_dot)
AC_SUBST(enable_html_docs)
AC_SUBST(enable_latex_docs)
])


dnl @synopsis AX_COMPARE_VERSION(VERSION_A, OP, VERSION_B, [ACTION-IF-TRUE], [ACTION-IF-FALSE])
dnl
dnl This macro compares two version strings.  It is used heavily in the
dnl macro _AX_PATH_BDB for library checking. Due to the various number
dnl of minor-version numbers that can exist, and the fact that string
dnl comparisons are not compatible with numeric comparisons, this is
dnl not necessarily trivial to do in a autoconf script.  This macro makes
dnl doing these comparisons easy.
dnl
dnl The six basic comparisons are available, as well as checking
dnl equality limited to a certain number of minor-version levels.
dnl
dnl The operator OP determines what type of comparison to do, and
dnl can be one of:
dnl  eq  - equal (test A == B)
dnl  ne  - not equal (test A != B)
dnl  le  - less than or equal (test A <= B)
dnl  ge  - greater than or equal (test A >= B)
dnl  lt  - less than (test A < B)
dnl  gt  - greater than (test A > B)
dnl
dnl Additionally, the eq and ne operator can have a number after it
dnl to limit the test to that number of minor versions.
dnl  eq0 - equal up to the length of the shorter version
dnl  ne0 - not equal up to the length of the shorter version
dnl  eqN - equal up to N sub-version levels
dnl  neN - not equal up to N sub-version levels
dnl
dnl When the condition is true, shell commands ACTION-IF-TRUE are run,
dnl otherwise shell commands ACTION-IF-FALSE are run.  The environment
dnl variable 'ax_compare_version' is always set to either 'true' or 'false'
dnl as well.
dnl
dnl Examples:
dnl   AX_COMPARE_VERSION([3.15.7],[lt],[3.15.8])
dnl   AX_COMPARE_VERSION([3.15],[lt],[3.15.8])
dnl would both be true.
dnl
dnl   AX_COMPARE_VERSION([3.15.7],[eq],[3.15.8])
dnl   AX_COMPARE_VERSION([3.15],[gt],[3.15.8])
dnl would both be false.
dnl
dnl   AX_COMPARE_VERSION([3.15.7],[eq2],[3.15.8])
dnl would be true because it is only comparing two minor versions.
dnl   AX_COMPARE_VERSION([3.15.7],[eq0],[3.15])
dnl would be true because it is only comparing the lesser number of
dnl minor versions of the two values.
dnl
dnl Note: The characters that separate the version numbers do not
dnl matter.  An empty string is the same as version 0.  OP is evaluated
dnl by autoconf, not configure, so must be a string, not a variable.
dnl
dnl The author would like to acknowledge Guido Draheim whose advice about
dnl the m4_case and m4_ifvaln functions make this macro only include
dnl the portions necessary to perform the specific comparison specified
dnl by the OP argument in the final configure script.
dnl
dnl @version $Id: ax_compare_version.m4,v 1.1 2004/03/01 19:14:43 guidod Exp $
dnl @author Tim Toolan <toolan@ele.uri.edu>
dnl

dnl #########################################################################
AC_DEFUN([AX_COMPARE_VERSION], [
  # Used to indicate true or false condition
  ax_compare_version=false

  # Convert the two version strings to be compared into a format that
  # allows a simple string comparison.  The end result is that a version
  # string of the form 1.12.5-r617 will be converted to the form
  # 0001001200050617.  In other words, each number is zero padded to four
  # digits, and non digits are removed.
  AS_VAR_PUSHDEF([A],[ax_compare_version_A])
  A=`echo "$1" | sed -e 's/\([[0-9]]*\)/Z\1Z/g' \
                     -e 's/Z\([[0-9]]\)Z/Z0\1Z/g' \
                     -e 's/Z\([[0-9]][[0-9]]\)Z/Z0\1Z/g' \
                     -e 's/Z\([[0-9]][[0-9]][[0-9]]\)Z/Z0\1Z/g' \
                     -e 's/[[^0-9]]//g'`

  AS_VAR_PUSHDEF([B],[ax_compare_version_B])
  B=`echo "$3" | sed -e 's/\([[0-9]]*\)/Z\1Z/g' \
                     -e 's/Z\([[0-9]]\)Z/Z0\1Z/g' \
                     -e 's/Z\([[0-9]][[0-9]]\)Z/Z0\1Z/g' \
                     -e 's/Z\([[0-9]][[0-9]][[0-9]]\)Z/Z0\1Z/g' \
                     -e 's/[[^0-9]]//g'`

  dnl # In the case of le, ge, lt, and gt, the strings are sorted as necessary
  dnl # then the first line is used to determine if the condition is true.
  dnl # The sed right after the echo is to remove any indented white space.
  m4_case(m4_tolower($2),
  [lt],[
    ax_compare_version=`echo "x$A
x$B" | sed 's/^ *//' | sort -r | sed "s/x${A}/false/;s/x${B}/true/;1q"`
  ],
  [gt],[
    ax_compare_version=`echo "x$A
x$B" | sed 's/^ *//' | sort | sed "s/x${A}/false/;s/x${B}/true/;1q"`
  ],
  [le],[
    ax_compare_version=`echo "x$A
x$B" | sed 's/^ *//' | sort | sed "s/x${A}/true/;s/x${B}/false/;1q"`
  ],
  [ge],[
    ax_compare_version=`echo "x$A
x$B" | sed 's/^ *//' | sort -r | sed "s/x${A}/true/;s/x${B}/false/;1q"`
  ],[
    dnl Split the operator from the subversion count if present.
    m4_bmatch(m4_substr($2,2),
    [0],[
      # A count of zero means use the length of the shorter version.
      # Determine the number of characters in A and B.
      ax_compare_version_len_A=`echo "$A" | awk '{print(length)}'`
      ax_compare_version_len_B=`echo "$B" | awk '{print(length)}'`

      # Set A to no more than B's length and B to no more than A's length.
      A=`echo "$A" | sed "s/\(.\{$ax_compare_version_len_B\}\).*/\1/"`
      B=`echo "$B" | sed "s/\(.\{$ax_compare_version_len_A\}\).*/\1/"`
    ],
    [[0-9]+],[
      # A count greater than zero means use only that many subversions
      A=`echo "$A" | sed "s/\(\([[0-9]]\{4\}\)\{m4_substr($2,2)\}\).*/\1/"`
      B=`echo "$B" | sed "s/\(\([[0-9]]\{4\}\)\{m4_substr($2,2)\}\).*/\1/"`
    ],
    [.+],[
      AC_WARNING(
        [illegal OP numeric parameter: $2])
    ],[])

    # Pad zeros at end of numbers to make same length.
    ax_compare_version_tmp_A="$A`echo $B | sed 's/./0/g'`"
    B="$B`echo $A | sed 's/./0/g'`"
    A="$ax_compare_version_tmp_A"

    # Check for equality or inequality as necessary.
    m4_case(m4_tolower(m4_substr($2,0,2)),
    [eq],[
      test "x$A" = "x$B" && ax_compare_version=true
    ],
    [ne],[
      test "x$A" != "x$B" && ax_compare_version=true
    ],[
      AC_WARNING([illegal OP parameter: $2])
    ])
  ])

  AS_VAR_POPDEF([A])dnl
  AS_VAR_POPDEF([B])dnl

  dnl # Execute ACTION-IF-TRUE / ACTION-IF-FALSE.
  if test "$ax_compare_version" = "true" ; then
    m4_ifvaln([$4],[$4],[:])dnl
    m4_ifvaln([$5],[else $5])dnl
  fi
]) dnl AX_COMPARE_VERSION


# ============ Test the nature of the va_list type. ==================
AC_DEFUN(CHECK_VA_LIST,
[
  # Check to see how the compiler defines va_list, as pointer or array.
  AC_MSG_CHECKING(whether va_lists can be copied by value)
  AC_CACHE_VAL(ac_cv_valistisarray,[
      AC_TRY_RUN([
          #include <stdarg.h>
          void f(int i, ...) {
              va_list args1, args2;
              va_start(args1, i);
              args2 = args1;
              if (va_arg(args2, int) != 42 || va_arg(args1, int) != 42)
                  exit(1);
              va_end(args1); va_end(args2);
          }
          int main() { f(0, 42); return(0); }],
          ac_cv_valistisarray=false,
          ac_cv_valistisarray=true,
          ac_cv_valistisarray=false
      )
  ])

  if test "$ac_cv_valistisarray" = true ; then
      AC_DEFINE(HAVE_VA_LIST_AS_ARRAY, [1], [Define if va_list is defined as an array])
      AC_MSG_RESULT(yes)
  else
      AC_MSG_RESULT(no)
  fi
])


# ==================== wxWidgets  =========================
AC_DEFUN([CHECK_WXWIDGETS],
[
    AC_MSG_CHECKING([for wxWidgets])
    AC_PATH_PROG([WXWIDGETS_CONFIG],wx-config)

    if test "x_$WXWIDGETS_CONFIG" != "x_"
    then
        wxCflags=`$WXWIDGETS_CONFIG --cflags`
        AC_SUBST(WXWIDGETS_CFLAGS,$wxCflags)
        wxCXXflags=`$WXWIDGETS_CONFIG --cxxflags`
        AC_SUBST(WXWIDGETS_CXXFLAGS,$wxCXXflags)
        wxlibs=`$WXWIDGETS_CONFIG --libs`
        AC_SUBST(WXWIDGETS_LIBS,$wxlibs)
        wxver=`$WXWIDGETS_CONFIG --version`

        AC_MSG_CHECKING([wxWidgets revision])
        AC_MSG_RESULT([found version $wxver])

        enable_wxwidgets=yes
    else
        enable_wxwidgets=no
        AC_MSG_WARN([no wx-config found - wxWidgets disabled])
    fi
])dnl


# ==================== named ====================
# Find the installed executable of named/bind.
AC_DEFUN([CHECK_NAMED],
[
    AC_ARG_WITH(named,
                [--with-named=PATH the named/bind executable],
                [named_program=$withval],
                [named_program=""],
	             )

    if test x_$named_program != x_; then
      AC_MSG_RESULT([Using named from --with-named $named_program])
      AC_SUBST(NAMED_PROGRAM, $named_program)
    else
      AC_PATH_PROG([NAMED_PROGRAM], [named], 
                   [named],
                   [$PATH:/sbin:/usr/sbin:/usr/local/sbin]
                   )
    fi
    
    if ! test -x $NAMED_PROGRAM; then
        AC_MSG_WARN([Cannot execute $NAMED_PROGRAM.  Tests that require it will not be executed.])
        NAMED_PROGRAM=""
    fi
])

