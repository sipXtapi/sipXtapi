##
## AC macros for general packages like OpenSSL, Xerces, etc
##

# ============= C P P U N I T ==================
AC_DEFUN([CHECK_CPPUNIT],
[
    ## Had trouble deciding whether to make this optional or not
    ## hard to imagine a situation where someone would want to 
    ## compile unittests so make it required for now
    AC_LANG_PUSH(C++)
    AC_CHECK_HEADER([cppunit/TestCase.h],,
        AC_MSG_ERROR([Cannot find required package cppunit to make unittests]))
    AC_LANG_POP(C++)
])


# ============ J B O S S  =======================
AC_DEFUN([CHECK_JBOSS_HOME],
[
   AC_ARG_VAR(JBOSS_HOME, [JBoss home directory])
   test -z $JBOSS_HOME && JBOSS_HOME=/usr/local/jboss;
   J2EE_JAR=$JBOSS_HOME/client/jboss-j2ee.jar
   AC_CHECK_FILE([$J2EE_JAR],
       AC_SUBST(J2EE_JAR), 
       AC_MSG_ERROR([You must install JBoss 2.4.4 with Tomcat and set JBOSS_HOME and TOMCAT_HOME home \
           env. variables to the jboss and catalina directory respectively]))
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
        AC_PATH_PROG(jar, tools.jar, ,$dir/lib)
        if test x$jar != x; then
            found_jdk="yes";
            JAVA_HOME=$dir
            break;
        fi
    done

    if test x_$found_jdk != x_yes; then
        AC_MSG_ERROR([Cannot find tools.jar in expected location. You may try setting the JAVA_HOME environment variable if you haven't already done so])
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
           CFLAGS="$CFLAGS $XFLAGS";
           CXXFLAGS="$CXXFLAGS $XFLAGS";

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
[   AC_MSG_CHECKING([for openssl])
    for dir in $withval /usr/local/ssl /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /usr; do
        if test -f "$dir/include/openssl/ssl.h"; then
            found_ssl="yes";
            ssldir="$dir"
            break;
        fi
        if test -f "$dir/include/ssl.h"; then
            found_ssl="yes";
            break;
        fi
    done
    
    test x_$found_ssl != x_yes && 
        AC_MSG_ERROR(Cannot find ssl libraries)

    printf "found in $ssldir\n"
    CFLAGS="$CFLAGS -I$ssldir/include -DHAVE_SSL"
    CXXFLAGS="$CXXFLAGS -I$ssldir/include -DHAVE_SSL"
    LDFLAGS="$LDFLAGS -L$ssldir/lib -lssl -lcrypto"
    HAVE_SSL=yes
    AC_SUBST(HAVE_SSL)
],
[
    AC_MSG_RESULT(yes)
])
])dnl


# ============ X E R C E S ==================
AC_DEFUN([CHECK_XERCES],
[   AC_MSG_CHECKING([for xerces])
    for dir in $withval /usr/local/xcercesc /usr/lib/xercesc /usr/xercesc /usr/pkg /usr/local /usr; do
        xercesdir="$dir"
        if test -f "$dir/include/xercesc/sax/Parser.hpp"; then
            found_xerces="yes";
            CFLAGS="$CFLAGS -I$xercesdir/include/xercesc";
            CXXFLAGS="$CXXFLAGS -I$xercesdir/include/xercesc";
            break;
        fi
        if test -f "$dir/include/sax/Parser.hpp"; then
            found_xerces="yes";
            CFLAGS="$CFLAGS -I$xercesdir/include";
            CXXFLAGS="$CXXFLAGS -I$xercesdir/include";
            break;
        fi
    done

    if test x_$found_xerces != x_yes; then
        AC_MSG_ERROR(Cannot find xerces libraries)
    else
        printf "found in $xercesdir\n";
        LIBS="$LIBS -lxerces-c";
        LDFLAGS="$LDFLAGS -L$xercesdir/lib";
    fi
],
[
    AC_MSG_RESULT(yes)
])
])dnl

# ============ M O D   C P L U S P L U S ==================
AC_DEFUN([CHECK_MODCPLUSPLUS],
[
    AC_MSG_CHECKING([for mod_cplusplus])
    AC_ARG_WITH(mod_cplusplus,
                [--with-mod_cplusplus=PATH to mod_cplusplus source directory],
                )        
    for dir in $withval /usr/local/apache2 /usr/local; do
        mod_cplusplusdir="$dir"
	    if test -f "$dir/include/mod_cplusplus.h";
        then 
            found_mod_cplusplus="yes";
            break;
        fi
    done
       
    if test x_$found_mod_cplusplus != x_yes; 
    then
        AC_MSG_ERROR([Cannot find mod_cplusplus])
    else
        AC_MSG_RESULT($mod_cplusplusdir)
    fi
])dnl

# ==================== C G I C C  =========================
AC_DEFUN([CHECK_CGICC],
[
    AC_MSG_CHECKING([for cgicc])
    AC_ARG_WITH(mod_cgicc,
                [--with-mod_cgicc=PATH to cgicc install directory],
                )        
    for dir in $withval /usr/local /usr/local/apache2 /usr; do
        cgiccdir="$dir"
	    if test -f "$dir/include/cgicc/Cgicc.h";
        then 
            found_cgicc="yes";
            break;
        fi
    done
       
    if test x_$found_cgicc != x_yes; 
    then
        AC_MSG_ERROR([Cannot find cgicc])
    else
        AC_MSG_RESULT($cgiccdir)
        AC_SUBST(CGICC_DIR, [$cgiccdir])
    fi
])dnl


# ============ A P A C H E 2 ==================
AC_DEFUN([CHECK_APACHE2],
[   AC_MSG_CHECKING([for apache2])
    AC_ARG_WITH(apache2,
                [--with-apache=PATH path to apache2 install directory],
                )        
    for dir in $withval /usr/local/apache2; do
	if test -f "$dir/include/httpd.h"; then
            found_apache2="yes";
            AC_SUBST(APACHE2_HOME, [$dir])
            break;
        fi
   done

   if test x_$found_apache2 != x_yes; then
       AC_MSG_ERROR(Cannot find apache2)
   else
       AC_MSG_RESULT($APACHE2_HOME)
       AC_SUBST(APACHE2_CFLAGS, -I$dir/include)
       AC_SUBST(APACHE2_CXXFLAGS, -I$dir/include)
   fi
])dnl

# ============ L I B W W W ==================
AC_DEFUN([CHECK_LIBWWW],
[   AC_MSG_CHECKING([for libwww])
    for dir in $withval /usr/local/w3c-libwww /usr/lib/w3c-libwww /usr/w3c-libwww /usr/pkg /usr/local /usr; do
        lwwwdir="$dir"
        if test -f "$dir/include/w3c-libwww/WWWLib.h"; then
            found_www="yes";
            CFLAGS="$CFLAGS -I$lwwwdir/include/w3c-libwww";
            CXXFLAGS="$CXXFLAGS -I$lwwwdir/include/w3c-libwww";
            break;
        fi
        if test -f "$dir/include/WWWlib.h"; then
            found_www="yes";
            CFLAGS="$CFLAGS -I$lwwwdir/include";
            CXXFLAGS="$CXXFLAGS -I$lwwwdir/include ";
            break;
        fi
    done
    if test x_$found_www != x_yes; then
        AC_MSG_ERROR(Cannot find libwww libraries)
    else
        printf "found in $lwwwdir\n";
        LIBS="$LIBS -lwwwapp -lwwwfile -lwwwhttp -lwwwssl -lwwwcore";

		LIBS="$LIBS -lwwwinit -lwwwapp -lwwwhttp -lwwwcache -lwwwcore";
	 	LIBS="$LIBS -lwwwfile -lwwwutils -lwwwmime -lwwwstream -lmd5";
		LIBS="$LIBS -lpics -lwwwnews -lwwwdir -lwwwtelnet -lwwwftp";
		LIBS="$LIBS -lwwwmux -lwwwhtml -lwwwgopher -lwwwtrans -lwwwzip";
        LIBS="$LIBS -lwwwssl -lwwwxml -lxmlparse -lxmltok";

        LDFLAGS="$LDFLAGS -L$lwwwdir/lib";
    fi
],
[
    AC_MSG_RESULT(yes)
])
])dnl

# ============ PCRE ==================
AC_DEFUN([CHECK_PCRE],
[   AC_MSG_CHECKING([for pcre])
    AC_ARG_WITH(pcre,
                [--with-apache=PATH path to pcre install directory],
                )        
    for dir in $withval /usr/local /usr/local/pcre /usr; do
	if test -f "$dir/include/pcre.h"; then
            found_pcre="yes";
            AC_SUBST(PCRE_HOME, [$dir])
            break;
        fi
   done

   if test x_$found_pcre != x_yes; then
       AC_MSG_ERROR(Cannot find pcre)
   else
       AC_MSG_RESULT($PCRE_HOME)
       AC_SUBST(PCRE_CFLAGS, -I$dir/include)
       AC_SUBST(PCRE_CXXFLAGS, -I$dir/include)
       AC_SUBST(PCRE_LIBS, -lpcre )
       AC_SUBST(PCRE_LDFLAGS, -L$dir/lib)
   fi
])dnl

