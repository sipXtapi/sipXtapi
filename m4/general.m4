##
## AC macros for general packages like OpenSSL, Xerces, etc
##

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
[
AC_CHECK_HEADER(
  [openssl/ssl.h],,
  [ AC_CHECK_HEADER(
      [openssl/ssl.h],,
      [ AC_CHECK_HEADER(
          [ssl.h],,
          [ AC_CHECK_HEADER(
              [ssl.h],,
              [ AC_MSG_ERROR([Failed to find OpenSSL (ssl.h)]) ],
              [krb5.h]
            )
          ]
        )
      ],
      [krb5.h]
    )
  ]
)
CPPFLAGS="${CPPFLAGS} -DHAVE_SSL"
])


# ============ C O R E A U D I O =======================

AC_DEFUN([CHECK_COREAUDIO],
[
    AC_MSG_CHECKING([for CoreAudio])

    if test "`uname`" == "Darwin"; then
        AC_SUBST(CA_LIBS,"-framework CoreAudio -framework AudioToolbox")
        AC_MSG_RESULT([yes])
        AM_CONDITIONAL([COREAUDIO_DRIVERS], [true])
    else
        AC_SUBST(CA_LIBS,"")
        AC_MSG_RESULT([not needed])
        AM_CONDITIONAL([COREAUDIO_DRIVERS], [false])
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
])

# ============ P C R E ==================
AC_DEFUN([CHECK_PCRE],
[   AC_MSG_CHECKING([for pcre >= 4.5])
    # Process the --with-pcre argument which gives the pcre base directory.
    AC_ARG_WITH(pcre,
                [  --with-pcre=PATH         path to pcre install directory],
                )
    homeval=$withval
    # Have to unset withval so we can tell if --with-pcre-includedir was
    # specified, as AC_ARG_WITH will not unset withval if the option is not
    # there!
    withval=

    # Process the --with-pcre-includedir argument which gives the pcre include
    # directory.
    AC_ARG_WITH(pcre-includedir,
                [  --with-pcre-includedir=PATH path to pcre include directory (containing pcre.h)],
                )
    # If withval is set, use that.  If not and homeval is set, use
    # $homeval/include.  If neither, use null.
    includeval=${withval:-${homeval:+$homeval/include}}
    withval=

    # Process the --with-pcre-libdir argument which gives the pcre library
    # directory.
    AC_ARG_WITH(pcre-libdir,
                [  --with-pcre-libdir=PATH  path to pcre lib directory (containing libpcre.{so,a})],
                )
    libval=${withval:-${homeval:+$homeval/lib}}

    # Check for pcre.h in the specified include directory if any, and a number
    # of other likely places.
    for dir in $includeval /usr/local/include /usr/local/pcre/include /usr/include /usr/include/pcre /sw/include /opt/local/include; do
        if test -f "$dir/pcre.h"; then
            found_pcre_include="yes";
            includeval=$dir
            break;
        fi
    done

    # Check for libpcre.{so,a} in the specified lib directory if any, and a
    # number of other likely places.
    for dir in $libval /usr/local/lib /usr/local/pcre/lib /usr/lib /usr/lib64 /opt/local/lib /sw/lib; do
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


# ============ G S M ==================
AC_DEFUN([AM_PATH_GSM],
[
    # Unset withval, as AC_ARG_WITH does not unset it
    withval=
    # Process the --with-gsm argument which gives the libgsm base directory.
    AC_ARG_WITH(gsm,
                [AS_HELP_STRING([--with-gsm=PATH],
                                [path to libgsm install directory])],
                [],
                [with_gsm=]
                )
    homeval=$withval
    # Have to unset withval so we can tell if --with-gsm-includedir was
    # specified, as AC_ARG_WITH will not unset withval if the option is not
    # there!
    withval=

    # Process the --with-gsm-includedir argument which gives the libgsm include
    # directory.
    AC_ARG_WITH(gsm-includedir,
                [AS_HELP_STRING([--with-gsm-includedir=PATH],
                                [path to libgsm include directory (containing gsm.h)])],
                )
    # If withval is set, use that.  If not and homeval is set, use
    # $homeval/include.  If neither, use null.
    includeval=${withval:-${homeval:+$homeval/inc}}
    withval=

    # Process the --with-gsm-libdir argument which gives the libgsm library
    # directory.
    AC_ARG_WITH(gsm-libdir,
                [AS_HELP_STRING([--with-gsm-libdir=PATH],
                                [path to libgsm lib directory (containing libgsm.{so,a})])],
                )
    libval=${withval:-${homeval:+$homeval/lib}}

    # Check for gsm.h in the specified include directory if any, and a number
    # of other likely places.
    for dir in $includeval /usr/local/include /usr/local/gsm/inc /usr/include /usr/include/gsm /sw/include; do
        if test -f "$dir/gsm.h"; then
            found_gsm_include="yes";
            includeval=$dir
            break;
        fi
    done

    # Check for libgsm.{so,a} in the specified lib directory if any, and a
    # number of other likely places.
    for dir in $libval /usr/local/lib /usr/local/gsm/lib /usr/lib /usr/lib64 /sw/lib; do
        if test -f "$dir/libgsm.so" -o -f "$dir/libgsm.a"; then
            found_gsm_lib="yes";
            libval=$dir
            break;
        fi
    done

    # Test that we've been able to find both directories, and set the various
    # makefile variables.
    if test x_$found_gsm_include != x_yes -o x_$found_gsm_lib != x_yes; then
        AC_MSG_RESULT(not found)
    else
        ## Test for version
        gsm_major_version=`grep "GSM_MAJOR" $includeval/gsm.h | \
               sed 's/^#define[ \t]\+GSM_MAJOR[ \t]\+\([0-9]\+\)/\1/'`
        gsm_minor_version=`grep "GSM_MINOR" $includeval/gsm.h | \
               sed 's/^#define[ \t]\+GSM_MINOR[ \t]\+\([0-9]\+\)/\1/'`
        gsm_patchlevel_version=`grep "GSM_PATCHLEVEL" $includeval/gsm.h | \
               sed 's/^#define[ \t]\+GSM_PATCHLEVEL[ \t]\+\([0-9]\+\)/\1/'`

        gsm_ver="$gsm_major_version.$gsm_minor_version.$gsm_patchlevel_version"
        AX_COMPARE_VERSION([$gsm_ver],[ge],[1.0.10])

        if test "x_$ax_compare_version" = "x_false"; then
            AC_MSG_RESULT(too old (found version $gsm_ver))
        else
            AC_MSG_RESULT($gsm_ver is ok)

	    # Enable this when we begin using config.h
            #AC_DEFINE(HAVE_GSM, [1], [Defined if libgsm is present])
	    CFLAGS="${CFLAGS} -DHAVE_GSM"
	    CXXFLAGS="${CXXFLAGS} -DHAVE_GSM"

            GSM_CFLAGS="-I$includeval"
            GSM_CXXFLAGS="-I$includeval"
            GSM_LIBS="-lgsm"
            GSM_LDFLAGS="-L$libval"

            GSM_TARGET="plggsm"

            PLUGINS="${PLUGINS} GSM"
        fi
    fi
    AC_SUBST(GSM_TARGET)
    AC_SUBST(GSM_CFLAGS)
    AC_SUBST(GSM_CXXFLAGS)
    AC_SUBST(GSM_LIBS)
    AC_SUBST(GSM_LDFLAGS)
])dnl

AC_DEFUN([CHECK_GSM],
[
    AC_MSG_CHECKING([for libgsm >= 1.0.10])

    AC_ARG_ENABLE(codec-gsm,
    [AS_HELP_STRING([--enable-codec-gsm],
                    [Enable support for GSM codec @<:@default=auto@:>@])],
    [ case "${enableval}" in
         auto) AM_PATH_GSM ;;
         yes) AM_PATH_GSM ;;
         no) AC_MSG_RESULT(disabled) ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-codec-gsm) ;;
      esac],
    [AM_PATH_GSM])

    AM_CONDITIONAL([BUILD_CODEC_GSM], [test ! -z "$GSM_TARGET"])
])dnl


# ============ S P E E X ==================
dnl
dnl Enables use of speex dsp specific code within the project.
dnl Callers of this macro need to check the contrib_speex_enabled shell variable
dnl and supply the AC_CONFIG_SUBDIRS line that configures the
dnl sipXmediaLib/contrib/libspeex package.  
dnl See CHECK_SPEEX for more information.
AC_DEFUN([ENABLE_SPEEX_DSP],
[
    AC_MSG_CHECKING([if speex dsp usage is enabled])
    speex_dsp_enabled=no;
    AC_ARG_ENABLE([speex-dsp],
       [AS_HELP_STRING([--enable-speex-dsp],
          [Enable SPEEX dsp library usage @<:@default=no@:>@])],
       [ case "${enableval}" in
            yes)  AC_MSG_RESULT(yes);
                  speex_dsp_enabled=yes ;;
            no) AC_MSG_RESULT(no) ;;
            *) AC_MSG_ERROR(bad value ${enableval} for --enable-speex-dsp) ;;
         esac],
       [AC_MSG_RESULT(no)])

    # Check to see if speex dsp was selected, and speex usage has not been
    # checked and configured
    if test "x$speex_dsp_enabled" == "xyes" -a "x$speex_detected" == "x"; then
        CHECK_SPEEX
    fi

    dnl now the important part of this macro...
    if test "x$speex_dsp_enabled" == "xyes"; then
        # Specify to enable speex dsp code
        CFLAGS="${CFLAGS} -DHAVE_SPEEX" ; CXXFLAGS="${CXXFLAGS} -DHAVE_SPEEX"
    fi
])dnl

AC_DEFUN([CHECH_DYNEXT],
[
    AC_MSG_CHECKING([for default shared object extenetion])

    case ${host} in
        *-apple-darwin*) DYNEXT=".dylib" ;;
        *)               DYNEXT=".so"    ;;
    esac
    
    AC_MSG_RESULT( ${DYNEXT} )

    AC_SUBST(DYNEXT)
])dnl


dnl
dnl Enables the speex codec plugin to be built and linked to in sipXmediaLib.
dnl Callers of this macro need to check the contrib_speex_enabled shell variable
dnl and supply the AC_CONFIG_SUBDIRS line that configures the
dnl sipXmediaLib/contrib/libspeex package.  
dnl See CHECK_SPEEX for more information.
AC_DEFUN([AM_SET_STATIC_SPEEX],
[
    CODEC_SPEEX_STATIC=true
    AM_SET_SPEEX
    AC_DEFINE(CODEC_SPEEX_STATIC, [1], [Select SPEEX codecs for static link])
    STATIC_CODEC_LIBS="${STATIC_CODEC_LIBS} mp/codecs/plgspeex/codec_speex.la"
    AC_SUBST(STATIC_CODEC_LIBS)
])dnl
AC_DEFUN([AM_SET_SPEEX],
[
    # Check to see if speex dsp was selected, and speex usage has not been
    # checked and configured
    if test "x$codec_speex_enabled" == "xyes" -a "x$speex_detected" == "x"; then
        CHECK_SPEEX
    fi
    PLUGINS="${PLUGINS} SPEEX"
    SPEEX_TARGET="plgspeex"
    AC_SUBST(SPEEX_TARGET)
])dnl
AC_DEFUN([ENABLE_CODEC_SPEEX],
[
    AC_ARG_ENABLE([codec-speex],
                  [AS_HELP_STRING([--enable-codec-speex],
                                  [Enable support for SPEEX codec @<:@default=no@:>@])],
                  [ case "${enableval}" in
                       static) AM_SET_STATIC_SPEEX ;;
                       yes) AM_SET_SPEEX ;;
                       no) ;;
                       *) AC_MSG_ERROR(bad value ${enableval} for --enable-codec-speex) ;;
                    esac])
    AM_CONDITIONAL([BUILD_CODEC_SPEEX], [test ! -z "$SPEEX_TARGET"])
    AM_CONDITIONAL(SPEEX_STATIC, test "$CODEC_SPEEX_STATIC" = true)
])dnl

dnl
dnl Enables speex support, checks to make sure that it is present,
dnl or needs to be built.  If it needs to be built, then the user that
dnl calls this needs to check the contrib_speex_enabled shell variable
dnl and supply the AC_CONFIG_SUBDIRS line that configures the
dnl sipXmediaLib/contrib/libspeex package.  This cannot be done within
dnl CHECK_SPEEX because it can only be specified once, and CHECK_SPEEX has the
dnl possibility of being specified more than once (and does in sipXmediaLib).
AC_DEFUN([CHECK_SPEEX],
[
    AC_MSG_CHECKING([for libspeex >= 1.2rc1])

    # Check if the user wished to force usage of contrib version of speex
    contrib_speex_enabled=no;
    AC_ARG_ENABLE([contrib-speex],
        [AS_HELP_STRING([--enable-contrib-speex],
            [Ignore any installed SPEEX libraries. Instead, build and use the ones in the contrib directory. @<:@default=no@:>@])],
        [ case "${enableval}" in
            yes) contrib_speex_enabled=yes ;;
            no) contrib_speex_enabled=no ;;
            *) AC_MSG_ERROR(bad value ${enableval} for --enable-contrib-speex) ;;
          esac],
        [contrib_speex_enabled=no])
    
    # Detect if speex is installed.
    speex_detected=no;
    if test "x$contrib_speex_enabled" == "xno"; then
        PKG_CHECK_MODULES([SPEEX],
                          [speex >= 1.2rc1],
                          PKG_CHECK_MODULES([SPEEXDSP],
                                  [speexdsp >= 1.2rc1],
                                  speex_detected=yes,
                                  speex_detected=no; contrib_speex_enabled=yes),
                          speex_detected=no; contrib_speex_enabled=yes)
        pkg_failed=no
        _PKG_CONFIG([SPEEX_LIBDIR], [variable=libdir], [speex >= 1.2rc1])
        if test "x$pkg_failed" = "xuntried"; then
            AC_MSG_ERROR(Failed to get the speex library directory from pkg-config!)
        else
            SPEEX_STATIC_LIB=${SPEEX_LIBDIR}/libspeex.a
            SPEEXDSP_STATIC_LIB=${SPEEX_LIBDIR}/libspeexdsp.a
        fi
    fi

    # if contrib speex is selected, use it.
    if test "x$contrib_speex_enabled" == "xyes" ; then
        AC_MSG_RESULT([using svn version])
        SPEEX_ROOT='${top_srcdir}/../sipXmediaLib/contrib/libspeex'
        SPEEX_CFLAGS="-I${SPEEX_ROOT}/include"
        SPEEX_CFLAGS+=' -I${top_builddir}/sipXmediaLib/contrib/libspeex/include'
        SPEEX_STATIC_LIB='${top_builddir}/sipXmediaLib/contrib/libspeex/libspeex/.libs/libspeex.a'
        SPEEX_LIBS=${SPEEX_STATIC_LIB}
        SPEEXDSP_STATIC_LIB='${top_builddir}/sipXmediaLib/contrib/libspeex/libspeex/.libs/libspeexdsp.a'
        SPEEXDSP_LIBS=${SPEEXDSP_STATIC_LIB}
        AC_SUBST(SPEEX_ROOT)
        AC_SUBST(SPEEX_CFLAGS)
        AC_SUBST(SPEEX_LIBS) 
        AC_SUBST(SPEEX_STATIC_LIB)
        AC_SUBST(SPEEXDSP_STATIC_LIB)
        AC_SUBST(SPEEXDSP_LIBS)
    elif test "x$speex_detected" == "xyes"; then
        AC_MSG_RESULT([ok])
        AC_SUBST(SPEEX_CFLAGS)
        AC_SUBST(SPEEX_LIBS)
        AC_SUBST(SPEEX_STATIC_LIB)
        AC_SUBST(SPEEXDSP_STATIC_LIB)
        AC_SUBST(SPEEXDSP_LIBS)
    else
        AC_MSG_ERROR([No speex found!])
    fi
])dnl

# ========== P C M A  P C M U =================
AC_DEFUN([AM_SET_STATIC_PCMA_PCMU],
[
    CODEC_PCMAPCMU_STATIC=true
    AM_SET_PCMA_PCMU
    AC_DEFINE(CODEC_PCMA_PCMU_STATIC, [1], [Select PCMA and PCMU codecs for static link])
    STATIC_CODEC_LIBS="${STATIC_CODEC_LIBS} mp/codecs/plgpcmapcmu/codec_pcmapcmu.la"
    AC_SUBST(STATIC_CODEC_LIBS)
])dnl
AC_DEFUN([AM_SET_PCMA_PCMU],
[
    PLUGINS="${PLUGINS} PCMA_PCMU"
    PCMAPCMU_TARGET="plgpcmapcmu"
    AC_SUBST(PCMAPCMU_TARGET)
])dnl
AC_DEFUN([CHECK_PCMA_PCMU],
[
    AC_ARG_ENABLE([codec-pcmapcmu],
                  [AS_HELP_STRING([--enable-codec-pcmapcmu],
                                  [Enable support for PCMA and PCMU codecs @<:@default=yes@:>@])],
                  [ case "${enableval}" in
                       static) AM_SET_STATIC_PCMA_PCMU ;;
                       auto) AM_SET_PCMA_PCMU ;;
                       yes) AM_SET_PCMA_PCMU ;;
                       no) AC_MSG_RESULT(Codecs PCMA & PCMU was disabled) ;;
                       *) AC_MSG_ERROR(bad value ${enableval} for --enable-codec-pcmapcmu) ;;
                    esac],
                  [AM_SET_PCMA_PCMU])
    AM_CONDITIONAL([BUILD_CODEC_PCMAPCMU], [test ! -z "$PCMAPCMU_TARGET"])
    AM_CONDITIONAL(PCMAPCMU_STATIC, test "$CODEC_PCMAPCMU_STATIC" = true)
])dnl

# ==============  T O N E S  ==================
AC_DEFUN([AM_SET_STATIC_TONES],
[
    CODEC_TONES_STATIC=true
    AM_SET_TONES
    AC_DEFINE(CODEC_TONES_STATIC, [1], [Select tones for static link])
    STATIC_CODEC_LIBS="${STATIC_CODEC_LIBS} mp/codecs/plgtones/codec_tones.la"
    AC_SUBST(STATIC_CODEC_LIBS)
])dnl
AC_DEFUN([AM_SET_TONES],
[
    PLUGINS="${PLUGINS} TONES"
    TONES_TARGET="plgtones"
    AC_SUBST(TONES_TARGET)
])dnl
AC_DEFUN([CHECK_TONES],
[
    AC_ARG_ENABLE([codec-tones],
                  [AS_HELP_STRING([--enable-codec-tones],
                                  [Enable support for Tones codec @<:@default=yes@:>@])],
                  [ case "${enableval}" in
                       static) AM_SET_STATIC_TONES ;;
                       auto) AM_SET_TONES ;;
                       yes) AM_SET_TONES ;;
                       no) AC_MSG_RESULT(Codec Tones was disabled) ;;
                       *) AC_MSG_ERROR(bad value ${enableval} for --enable-codec-tones) ;;
                    esac],
                  [AM_SET_TONES])
    AM_CONDITIONAL([BUILD_CODEC_TONES], [test ! -z "$TONES_TARGET"])
    AM_CONDITIONAL(TONES_STATIC, test "$CODEC_TONES_STATIC" = true)
])dnl

# ==============  L 1 6  ==================
AC_DEFUN([AM_SET_STATIC_L16],
[
    CODEC_L16_STATIC=true
    AM_SET_L16
    AC_DEFINE(CODEC_L16_STATIC, [1], [Select l16 for static link])
    STATIC_CODEC_LIBS="${STATIC_CODEC_LIBS} mp/codecs/plgl16/codec_l16.la"
    AC_SUBST(STATIC_CODEC_LIBS)
])dnl
AC_DEFUN([AM_SET_L16],
[
    PLUGINS="${PLUGINS} L16"
    L16_TARGET="plgl16"
    AC_SUBST(L16_TARGET)
])dnl
AC_DEFUN([CHECK_L16],
[
    AC_ARG_ENABLE([codec-l16],
                  [AS_HELP_STRING([--enable-codec-l16],
                                  [Enable support for L16 codec @<:@default=no@:>@])],
                  [ case "${enableval}" in
                       static) AM_SET_STATIC_L16 ;;
                       auto) AC_MSG_RESULT(Codec L16 was disabled) ;;
                       yes) AM_SET_L16 ;;
                       no) AC_MSG_RESULT(Codec L16 was disabled) ;;
                       *) AC_MSG_ERROR(bad value ${enableval} for --enable-codec-l16) ;;
                    esac],
                  [AM_SET_L16])
    AM_CONDITIONAL([BUILD_CODEC_L16], [test ! -z "$L16_TARGET"])
    AM_CONDITIONAL(L16_STATIC, test "$CODEC_L16_STATIC" = true)
])dnl

# =============== I L B C =====================

AC_DEFUN([AM_SET_ILBC],
[
# Currently only iLBC in contrib supported
    PLUGINS="${PLUGINS} iLBC"

    ILBC_INCLUDE="-I${PWD}/sipXmediaLib/contrib/libilbc/include"
    ILBC_LIB_ROOT="${PWD}/sipXmediaLib/contrib/libilbc/"
    ILBC_LIB_TARGET="lib/libilbc.a"
    ILBC_TARGET="plgilbc"
    AC_SUBST(ILBC_INCLUDE)
    AC_SUBST(ILBC_LIB_ROOT)    
    AC_SUBST(ILBC_LIB_TARGET)    
    AC_SUBST(ILBC_TARGET)    
    
])dnl
AC_DEFUN([CHECK_ILBC],
[
    AC_ARG_ENABLE([codec-ilbc],
                  [AS_HELP_STRING([--enable-codec-ilbc],
                                  [Enable support for ilbc codec @<:@default=yes@:>@])],
                  [ case "${enableval}" in
                       auto) AM_SET_ILBC ;;
                       yes) AM_SET_ILBC ;;
                       no) AC_MSG_RESULT(Codec iLBC was disabled) ;;
                       *) AC_MSG_ERROR(bad value ${enableval} for --enable-codec-ilbc) ;;
                    esac],
                  [AM_SET_ILBC])
    AM_CONDITIONAL([BUILD_CODEC_ILBC], [test ! -z "$ILBC_TARGET"])
])dnl


# == D E C L A R E _ C O D E C S _ S T A F F ==
AC_DEFUN([DECLARE_CODECS_STAFF],
[
    AC_SUBST([codeclibdir], ['${pkglibdir}/codecs'])

    CPPFLAGS="${CPPFLAGS} -DDEFAULT_CODECS_PATH='\"${codeclibdir}\"' "
])dnl


AC_DEFUN([CHECK_SPANDSP],
[
    AC_MSG_CHECKING([for libspandsp >= 0.0.2pre26])

    # Unset withval, as AC_ARG_WITH does not unset it
    withval=
    AC_ARG_WITH(spandsp-includedir,
                [AS_HELP_STRING([--with-spandsp-includedir=PATH],
                                [path to libspandsp include directory (containing spandsp.h)])],
                )
    # If withval is set, use that.  If not and homeval is set, use
    # $homeval/include.  If neither, use null.
    includeval=${withval}
    withval=

    # Process the --with-spansdp-libdir argument which gives the libgsm library
    # directory.
    AC_ARG_WITH(spandsp-libdir,
                [AS_HELP_STRING([--with-spandsp-libdir=PATH],
                                [path to libspandsp lib directory (containing libspandsp.{so,la})])],
                )
    libval=${withval}

    # Check for spansdp.h in the specified include directory if any, and a number
    # of other likely places.
    for dir in $includeval /usr/local/include /usr/include /sw/include; do
        if test -f "$dir/spandsp.h"; then
            found_spandsp_include="yes";
            includeval=$dir
            break;
        fi
    done

    # Check for libspansdp.{so,a} in the specified lib directory if any, and a
    # number of other likely places.
    for dir in $libval /usr/local/lib /usr/lib /usr/lib64 /sw/lib; do
        if test -f "$dir/libspandsp.so" -o -f "$dir/libspandsp.la"; then
            found_spandsp_lib="yes";
            libval=$dir
            break;
        fi
    done

    # Test that we've been able to find both directories, and set the various
    # makefile variables.
    if test x_$found_spandsp_include != x_yes -o x_$found_spandsp_lib != x_yes; then
        AC_MSG_RESULT(not found)
    else
        # testing for proper version
        ac_spandsp_ok=false
        ac_spandsp_new=false
        ac_spandsp_g726valid=false
        
        if test -f "$includeval/spandsp/g722.h" -o -f "$includeval/spandsp/g726.h"; then

            OLD_CFLAGS=$CFLAGS
            OLD_LDFLAGS=$LDFLAGS
            CFLAGS+=" -I$includeval"
            LDFLAGS+=" -L$libval  -lspandsp"

            AC_TRY_COMPILE([
                    #include <stdint.h>
                    #include <spandsp/bitstream.h>
                    #include <spandsp/g722.h>
                    #include <spandsp/g726.h>
                ],[
                    void* p = g726_init(0, 16000, G726_ENCODING_LINEAR, G726_PACKING_LEFT);
                    void* q = g722_encode_init(0, 64000, 0);
                    return p!=q;
                ],
                ac_libspandsp_newstyle=true,
                ac_libspandsp_newstyle=false)
            
            if test "$ac_libspandsp_newstyle" = false; then
                AC_TRY_COMPILE([
                        #include <stdint.h>
                        #include <spandsp/telephony.h>
                        #include <spandsp/g722.h>
                        #include <spandsp/g726.h>
                    ],[
                        void* p = g726_init(0, 16000, G726_ENCODING_LINEAR, G726_PACKING_LEFT);
                        void* q = g722_encode_init(0, 64000, 0);
                        return p!=q;
                    ],
                    ac_libspandsp_newstyle=true,
                    ac_libspandsp_newstyle=false)
             fi
            
                CFLAGS=$OLD_CFLAGS
                LDFLAGS=$OLD_LDFLAGS

            if test "$ac_libspandsp_newstyle" = false; then

                AC_MSG_WARN(ac_libspandsp_newstyle == false)
                OLD_CFLAGS=$CFLAGS
                OLD_LDFLAGS=$LDFLAGS
                CFLAGS+=" -I$includeval"
                LDFLAGS+=" -L$libval  -lspandsp"

                    AC_TRY_COMPILE([
                        #include <stdint.h>
                        #include <spandsp/g722.h>
                        #include <spandsp/g726.h>
                    ],[
                        void* p = g726_init(0, 16000, G726_ENCODING_LINEAR, 1);
                        void* q = g722_encode_init(0, 64000, 0);
                        return p!=q;
                    ],
                    ac_libspandsp_oldstyle=true,
                    ac_libspandsp_oldstyle=false)

                CFLAGS=$OLD_CFLAGS
                LDFLAGS=$OLD_LDFLAGS
                
                AC_MSG_WARN(ac_libspandsp_oldstyle = $ac_libspandsp_oldstyle)

                if test "$ac_libspandsp_oldstyle" = true; then
                    AC_DEFINE(HAVE_OLD_LIBSPANDSP, [1], [Have old version of libspandsp])
                    ac_spandsp_ok=true
                fi
            else
                ac_spandsp_ok=true
            fi
        fi
    
        if test "$ac_spandsp_ok" = true -a "$ac_libspandsp_newstyle" = true; then
            ac_spandsp_new=true
            ac_spandsp_g726valid=true
        fi
    
        if test "$ac_spandsp_ok" = true; then
            if test "$ac_spandsp_new" = true; then
                AC_MSG_RESULT(ok)
            else
                AC_MSG_RESULT(old version of libspandsp)
            fi
            SPANDSP_CFLAGS="-I$includeval"
            SPANDSP_CXXFLAGS="-I$includeval"
            SPANDSP_LIBS="-lspandsp"
            SPANDSP_LDFLAGS="-L$libval"
        else
            AC_MSG_ERROR(invalid version of spandsplib)
        fi


        if test "$ac_spandsp_ok" = true -a "$ac_spandsp_new" = false; then
                withval=
                AC_ARG_WITH(spandsp-validg726,
                [AS_HELP_STRING([--with-spandsp-validg726=VAL],
                                [Set VAL to 'yes' or 'ok' to use G726 with old libspandsp (where packing mode parameter compiled in library)])],
                )
            
            if test "$withval" = "yes" -o  "$withval" = "ok" ; then
                ac_spandsp_g726valid=true
            else
                # Need to check G726 packing
                AC_MSG_CHECKING([for libspandsp G726 packing])
                
                OLD_CFLAGS=$CFLAGS
                OLD_LDFLAGS=$LDFLAGS
                CFLAGS+=" $SPANDSP_CFLAGS"
                LDFLAGS+=" $SPANDSP_LDFLAGS $SPANDSP_LIBS"
                
                AC_TRY_RUN([
                        #include <stdint.h>
                        #include <spandsp/g722.h>
                        #include <spandsp/g726.h>
                        
                        int main() {
                        void* p = g726_init(0, 16000, G726_ENCODING_LINEAR, 1);
                        uint16_t data[16];
                        const uint16_t vdata[4] = {0xc, 0xc, 0xc, 0x3c};
                        uint8_t packed[4] = {1, 54};
                        int len, i;
                        
                        len = g726_decode(p, data, packed, 2);
                        
                        for (i = 0; i < 4; i++) {
                           if ( data[i] != vdata[i] )
                              return -1;
                        }
                        return 0;
                        }
                    ],
                    ac_spandsp_g726valid=true,
                    ac_spandsp_g726valid=false,
                    ac_spandsp_g726valid=false)


                CFLAGS=$OLD_CFLAGS
                LDFLAGS=$OLD_LDFLAGS

                if test "$ac_spandsp_g726valid" = true; then
                    AC_MSG_RESULT(ok)
                else
                    AC_MSG_RESULT(incorrect)
                fi
            fi
            
        fi
        
    fi
    AC_SUBST(SPANDSP_CFLAGS)
    AC_SUBST(SPANDSP_CXXFLAGS)
    AC_SUBST(SPANDSP_LIBS)
    AC_SUBST(SPANDSP_LDFLAGS)

])dnl

AC_DEFUN([EXTERNAL_EXTENITIONS],
[

# RTL Lib
    RTL_LDFLAGS=""
    RTL_CXXFLAGS=""
    
# RtlAudio    
    withval=
    AC_ARG_WITH(rtllibaudio,
                [AS_HELP_STRING([--with-rtllibaudio=library],
                                [Use specified library for RtlAudio])],
                )
    ac_external_rtlaudio=${withval}
    if test x${ac_external_rtlaudio} != x; then
    	RTL_LDFLAGS+=" -l${ac_external_rtlaudio} "
    fi


    withval=
    AC_ARG_WITH(rtllibaudio-path,
                [AS_HELP_STRING([--with-rtllibaudio-path=PATH],
                                [Add path for RtlAudio])],
                )
    ac_external_rtlaudio_path=${withval}
    if test x${ac_external_rtlaudio_path} != x; then
    	RTL_LDFLAGS+=" -L${ac_external_rtlaudio_path} -Wl,--rpath -Wl,${ac_external_rtlaudio_path} "
    fi

    withval=
    AC_ARG_WITH(rtllibaudio-inc-path,
                [AS_HELP_STRING([--with-rtllibaudio-inc-path=PATH],
                                [Add path for headers for RtlAudio])],
                )
    ac_external_rtlaudio_inc_path=${withval}
    if test x${ac_external_rtlaudio_inc_path} != x; then
    	RTL_CXXFLAGS+=" -I${ac_external_rtlaudio_inc_path} "
    fi

# Rtl
    withval=
    AC_ARG_WITH(rtllib,
                [AS_HELP_STRING([--with-rtllib=library],
                                [Use specified library for Rtl])],
                )
    ac_external_rtl=${withval}
    if test x${ac_external_rtl} != x; then
    	RTL_LDFLAGS+=" -l${ac_external_rtl} "
    fi

    
    withval=
    AC_ARG_WITH(rtllib-path,
                [AS_HELP_STRING([--with-rtllib-path=PATH],
                                [Add path for Rtl])],
                )
    ac_external_rtl_path=${withval}
    if test x${ac_external_rtl_path} != x; then
    	RTL_LDFLAGS+=" -L${ac_external_rtl_path} -Wl,--rpath -Wl,${ac_external_rtl_path} "
    fi
    
    withval=
    AC_ARG_WITH(rtllib-inc-path,
                [AS_HELP_STRING([--with-rtllib-inc-path=PATH],
                                [Add path for headers for Rtl])],
                )
    ac_external_rtl_inc_path=${withval}
    if test x${ac_external_rtl_inc_path} != x; then
    	RTL_CXXFLAGS+=" -I${ac_external_rtl_inc_path} "
    fi

# Rtl Defines
    AC_ARG_ENABLE([external-rtl-init],
                  [AS_HELP_STRING([--enable-external-rtl-init],
                                  [Initialize external RTL collector to log to sipXmediaLib internal one @<:@default=no@:>@])],
                  [ case "${enableval}" in
                       yes) ac_do_rtinit_define=true  ;;
                       no)  ac_do_rtinit_define=false ;;
                       *) AC_MSG_ERROR(bad value ${enableval} for --enable-external-rtl-init) ;;
                    esac],
                  [ac_do_rtinit_define=false ])
    if test x${ac_do_rtinit_define} != xfalse; then
	AC_DEFINE(INIT_EXTERNAL_RTL_COLLECTOR, [1], [Initialize external RTL collector with sipXmediaLib s internal one])
	RTL_CXXFLAGS+=" -DINIT_EXTERNAL_RTL_COLLECTOR "
    fi
    
    AC_ARG_ENABLE([rtl],
                  [AS_HELP_STRING([--enable-rtl],
                                  [Enable RTL for the whole library @<:@default=no@:>@])],
                  [ case "${enableval}" in
                       yes) ac_do_rtl_define=true  ;;
                       no)  ac_do_rtl_define=false ;;
                       *) AC_MSG_ERROR(bad value ${enableval} for --enable-rtl) ;;
                    esac],
                  [ac_do_rtl_define=false ])
    if test x${ac_do_rtl_define} != xfalse; then
	AC_DEFINE(RTL_ENABLED, [1], [Enable RTL for the whole library])
	RTL_CXXFLAGS+=" -DRTL_ENABLED "
    fi

    AC_ARG_ENABLE([rtlaudio],
                  [AS_HELP_STRING([--enable-rtlaudio],
                                  [Enable RTL_AUDIO for the whole library @<:@default=no@:>@])],
                  [ case "${enableval}" in
                       yes) ac_do_rtlaudio_define=true  ;;
                       no)  ac_do_rtlaudio_define=false ;;
                       *) AC_MSG_ERROR(bad value ${enableval} for --enable-rtlaudio) ;;
                    esac],
                  [ac_do_rtlaudio_define=false ])
    if test x${ac_do_rtlaudio_define} != xfalse; then
	AC_DEFINE(RTL_AUDIO_ENABLED, [1], [Enable RTL_AUDIO for the whole library])    
	RTL_CXXFLAGS+=" -DRTL_AUDIO_ENABLED "
    fi
])dnl

AC_DEFUN([EXTERNAL_EXTENITIONS_MEDIA],
[

# VAD
    withval=
    AC_ARG_WITH(external-vad,
                [AS_HELP_STRING([--with-external-vad=library],
                                [Use external library for VAD])],
                )
    ac_external_vad=${withval}

    withval=
    AC_ARG_WITH(external-vad-path,
                [AS_HELP_STRING([--with-external-vad-path=PATH],
                                [Add path for searching external library for VAD])],
                )
    ac_external_vad_path=${withval}

    if test x${ac_external_vad} != x; then
	AC_DEFINE(EXTERNAL_VAD, [1], [Use external library for VAD])
	EVAD_LDFLAGS="-l${ac_external_vad} "
	if test x${ac_external_vad_path} != x; then
		EVAD_LDFLAGS+=" -L${ac_external_vad_path} -Wl,--rpath -Wl,${ac_external_vad_path} "
	fi

        LDFLAGS+=" $EVAD_LDFLAGS "
        CXXFLAGS+=" -DEXTERNAL_VAD "	
    fi
    AC_SUBST(EVAD_LDFLAGS)

# PLC
    withval=
    AC_ARG_WITH(external-plc,
                [AS_HELP_STRING([--with-external-plc=library],
                                [Use external library for PLC])],
                )
    ac_external_plc=${withval}

    withval=
    AC_ARG_WITH(external-plc-path,
                [AS_HELP_STRING([--with-external-plc-path=PATH],
                                [Add path for searching external library for PLC])],
                )
    ac_external_plc_path=${withval}

    if test x${ac_external_plc} != x; then
	AC_DEFINE(EXTERNAL_PLC, [1], [Use external library for PLC])
	EPLC_LDFLAGS="-l${ac_external_plc} "
	if test x${ac_external_plc_path} != x; then
		EPLC_LDFLAGS+=" -L${ac_external_plc_path} -Wl,--rpath -Wl,${ac_external_plc_path} "
	fi

        LDFLAGS+=" $EPLC_LDFLAGS "
        CXXFLAGS+=" -DEXTERNAL_PLC "	
    fi
    AC_SUBST(EPLC_LDFLAGS)

# Jitter buffer estimation
    withval=
    AC_ARG_WITH(external-jbe,
                [AS_HELP_STRING([--with-external-jbe=library],
                                [Use external library for Jitter buffer estimation])],
                )
    ac_external_jbe=${withval}

    withval=
    AC_ARG_WITH(external-jbe-path,
                [AS_HELP_STRING([--with-external-jbe-path=PATH],
                                [Add path for searching external library for JBE])],
                )
    ac_external_jbe_path=${withval}

    if test x${ac_external_jbe} != x; then
	AC_DEFINE(EXTERNAL_JB_ESTIMATION, [1], [Use external library for jitter buffer estimation])
	EJBE_LDFLAGS="-l${ac_external_jbe} "
	if test x${ac_external_jbe_path} != x; then
		EJBE_LDFLAGS+=" -L${ac_external_jbe_path} -Wl,--rpath -Wl,${ac_external_jbe_path} "
	fi

        LDFLAGS+=" $EJBE_LDFLAGS "
        CXXFLAGS+=" -DEXTERNAL_JB_ESTIMATION "
    fi
    AC_SUBST(EJBE_LDFLAGS)

# AGC
    withval=
    AC_ARG_WITH(external-agc,
                [AS_HELP_STRING([--with-external-agc=library],
                                [Use external library for AGC])],
                )
    ac_external_agc=${withval}

    withval=
    AC_ARG_WITH(external-agc-path,
                [AS_HELP_STRING([--with-external-agc-path=PATH],
                                [Add path for searching external library for AGC])],
                )
    ac_external_agc_path=${withval}

    if test x${ac_external_agc} != x; then
       AC_DEFINE(EXTERNAL_AGC, [1], [Use external library for AGC])
       EAGC_LDFLAGS="-l${ac_external_agc} "
       if test x${ac_external_agc_path} != x; then
               EAGC_LDFLAGS+=" -L${ac_external_agc_path} -Wl,--rpath -Wl,${ac_external_agc_path} "
       fi

        LDFLAGS+=" $EAGC_LDFLAGS "
        CXXFLAGS+=" -DEXTERNAL_AGC "
    fi
    AC_SUBST(EAGC_LDFLAGS)
# Speaker selection
    withval=
    AC_ARG_WITH(external-ss,
                [AS_HELP_STRING([--with-external-ss=library],
                                [Use external library for speaker selection])],
                )
    ac_external_ss=${withval}

    withval=
    AC_ARG_WITH(external-ss-path,
                [AS_HELP_STRING([--with-external-ss-path=PATH],
                                [Add path for searching external library for SS])],
                )
    ac_external_ss_path=${withval}

    if test x${ac_external_ss} != x; then
	AC_DEFINE(EXTERNAL_SS, [1], [Use external library for speaker selection])
	SS_LDFLAGS="-l${ac_external_ss} "
	if test x${ac_external_ss_path} != x; then
		SS_LDFLAGS+=" -L${ac_external_ss_path} -Wl,--rpath -Wl,${ac_external_ss_path} "
	fi

        LDFLAGS+=" $SS_LDFLAGS "
        CXXFLAGS+=" -DEXTERNAL_SS "
    fi
    AC_SUBST(SS_LDFLAGS)
                                                                        

    CXXFLAGS+=" $RTL_CXXFLAGS "
    LDFLAGS+=" ${RTL_LDFLAGS} "
    AC_SUBST(RTL_CXXFLAGS)
    AC_SUBST(RTL_LDFLAGS)
    
])dnl



AC_DEFUN([SPEACKER_SELECTION_CHECK],
[
# Speaker selection enable
    AC_ARG_ENABLE([speaker-selection],
                  [AS_HELP_STRING([--enable-speaker-selection],
                                  [Enable Speaker Selection resource in flowgraph @<:@default=no@:>@])],
                  [ case "${enableval}" in
                       yes) ac_enable_speaker_selection=true  ;;
                       no)  ac_enable_speaker_selection=false ;;
                       *) AC_MSG_ERROR(bad value ${enableval} for --enable-speaker-selection) ;;
                    esac],
                  [ac_enable_speaker_selection=false ])
    if test x${ac_enable_speaker_selection} != xfalse; then
	AC_DEFINE(INSERT_SPEAKER_SELECTOR, [1], [Enable Sepaker Selection resource in flowgraph])
	CXXFLAGS+=" -DINSERT_SPEAKER_SELECTOR "
    fi


])dnl

# =============== G726  =====================
AC_DEFUN([AM_SET_STATIC_G726],
[
    CODEC_G726_STATIC=true
    AM_SET_G726
    if test "$G726_TARGET" != ""; then
    AC_DEFINE(CODEC_G726_STATIC, [1], [Select G726 for static link])

    LDFLAGS="$LDFLAGS $SPANDSP_LDFLAGS $SPANDSP_LIBS"
    STATIC_CODEC_LIBS="${STATIC_CODEC_LIBS} mp/codecs/plgg726/codec_g726.la"
    AC_SUBST(STATIC_CODEC_LIBS)
    fi
    
])dnl
AC_DEFUN([AM_SET_G726],
[
    if test x_$SPANDSP_CFLAGS != x_ -a "$ac_spandsp_g726valid" = true; then
        PLUGINS="${PLUGINS} G.726"
        G726_TARGET="plgg726"
    fi
    AC_SUBST(G726_TARGET)    
])dnl
AC_DEFUN([CHECK_G726],
[
    AC_ARG_ENABLE([codec-g726],
                  [AS_HELP_STRING([--enable-codec-g726],
                                  [Enable support for g726 codec @<:@default=yes@:>@])],
                  [ case "${enableval}" in
                       static) AM_SET_STATIC_G726 ;;
                       auto) AM_SET_G726 ;;
                       yes) AM_SET_G726 ;;
                       no) AC_MSG_RESULT(Codec G.726 was disabled) ;;
                       *) AC_MSG_ERROR(bad value ${enableval} for --enable-codec-g726) ;;
                    esac],
                  [AM_SET_G726])
    AM_CONDITIONAL([BUILD_CODEC_G726], [test ! -z "$G726_TARGET"])
    AM_CONDITIONAL(G726_STATIC, test "$CODEC_G726_STATIC" = true)
])dnl

# =============== G722  =====================
AC_DEFUN([AM_SET_STATIC_G722],
[
    CODEC_G722_STATIC=true
    AM_SET_G722
    if test "$G722_TARGET" != ""; then
    AC_DEFINE(CODEC_G722_STATIC, [1], [Select G722 for static link])

    LDFLAGS="$LDFLAGS $SPANDSP_LDFLAGS $SPANDSP_LIBS"
    STATIC_CODEC_LIBS="${STATIC_CODEC_LIBS} mp/codecs/plgg722/codec_g722.la"
    AC_SUBST(STATIC_CODEC_LIBS)
    fi
])dnl
AC_DEFUN([AM_SET_G722],
[
# Currently only iLBC in contrib supported
    if test x_$SPANDSP_CFLAGS != x_; then
        PLUGINS="${PLUGINS} G.722"
        G722_TARGET="plgg722"
    fi
    AC_SUBST(G722_TARGET)    
])dnl
AC_DEFUN([CHECK_G722],
[
    AC_ARG_ENABLE([codec-g722],
                  [AS_HELP_STRING([--enable-codec-g722],
                                  [Enable support for g722 codec @<:@default=yes@:>@])],
                  [ case "${enableval}" in
                       static) AM_SET_STATIC_G722 ;;
                       auto) AM_SET_G722 ;;
                       yes) AM_SET_G722 ;;
                       no) AC_MSG_RESULT(Codec G.722 was disabled) ;;
                       *) AC_MSG_ERROR(bad value ${enableval} for --enable-codec-g722) ;;
                    esac],
                  [AM_SET_G722])
    AM_CONDITIONAL([BUILD_CODEC_G722], [test ! -z "$G722_TARGET"])
    AM_CONDITIONAL(G722_STATIC, test "$CODEC_G722_STATIC" = true)
])dnl

AC_DEFUN([CHECK_GRAPH_INTERFACE],
[
    AC_ARG_ENABLE([topology-graph],
		  [AC_HELP_STRING([--enable-topology-graph],
				  [Enable toplogy graph as default processing @<:@default=no@:>@])],
		  [ case "${enableval}" in 
			yes) INTERFACE_FLAGS=" -DENABLE_TOPOLOGY_FLOWGRAPH_INTERFACE_FACTORY "
			     INTERFACE_FLAGS+="-DDISABLE_DEFAULT_PHONE_MEDIA_INTERFACE_FACTORY " ;;
			no)  INTERFACE_FLAGS=" " ;;
			*) AC_MSG_ERROR(bad value ${enableval} for --enable-topology-graph) ;;
		    esac],
		  [ enableval= ])
					       
    AC_SUBST(INTERFACE_FLAGS)    
])dnl

# === CHECK_OPUS
AC_DEFUN([AM_SET_STATIC_OPUS],
[
    CODEC_OPUS_STATIC=true
    AM_SET_OPUS
    if test "$OPUS_TARGET" != ""; then
        AC_DEFINE(CODEC_OPUS_STATIC, [1], [Select Opus for static link])
        LDFLAGS="$LDFLAGS"
        STATIC_CODEC_LIBS="${STATIC_CODEC_LIBS} mp/codecs/plgopus/codec_opus.la"
        AC_SUBST(STATIC_CODEC_LIBS)
    fi

])dnl
AC_DEFUN([AM_SET_OPUS],
[
    PLUGINS="${PLUGINS} OPUS"
    OPUS_TARGET="plgopus"
    AC_SUBST(OPUS_TARGET)
    OPUS_CFLAGS="-I/usr/include/opus"
    OPUS_LIBS="-lopus"
    AC_SUBST(OPUS_CFLAGS)
    AC_SUBST(OPUS_LIBS)
])dnl
AC_DEFUN([CHECK_OPUS],
[
    AC_ARG_ENABLE([codec-opus],
                  [AS_HELP_STRING([--enable-codec-opus],
                                  [Enable support for Opus codec @<:@default=yes@:>@])],
                  [ case "${enableval}" in
                       static) AM_SET_STATIC_OPUS ;;
                       auto) AM_SET_OPUS ;;
                       yes) AM_SET_OPUS ;;
                       no) AC_MSG_RESULT(Codec Opus was disabled) ;;
                       *) AC_MSG_ERROR(bad value ${enableval} for --enable-codec-opus) ;;
                    esac],
                  [AM_SET_OPUS])
    AM_CONDITIONAL([BUILD_CODEC_OPUS], [test ! -z "$OPUS_TARGET"])
    AM_CONDITIONAL(OPUS_STATIC, test "$CODEC_OPUS_STATIC" = true)
])dnl

# === CHECK_STREAM_PLAYER
AC_DEFUN([CHECK_STREAM_PLAYER],
[
    AC_ARG_ENABLE([stream-player],
        [AC_HELP_STRING([--disable-stream-player],
            [Disable stream player (removes sipXtack dependency on media layer) @<:@default=no@:>@])],
            [ case "${enableval}" in
                no)  disable_stream_player=true ;;
                yes) disable_stream_player=false ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --disable-media-player) ;;
                esac],
            [ disable_stream_player=false ])
    AM_CONDITIONAL(DISABLE_STREAM_PLAYER, test x$disable_stream_player = xtrue)
])dnl

# === AMR AMR_WB
AC_DEFUN([AM_SET_AMR],
[
# Currently only iLBC in contrib supported
    PLUGINS="${PLUGINS} AMR"
    AMR_TARGET="plgamr"
    AMRNB_INCLUDE="-I${PWD}/sipXmediaLib/contrib/libamrnb/"
    AMRNB_LIB_ROOT="${PWD}/sipXmediaLib/contrib/libamrnb/"    
    AC_SUBST(AMR_TARGET)    
    AC_SUBST(AMRNB_INCLUDE)    
    AC_SUBST(AMRNB_LIB_ROOT)

    # amr narrowband codec has it's own configure, 
    # so be sure to call it.
    AC_CONFIG_SUBDIRS([sipXmediaLib/contrib/libamrnb/])
])dnl

AC_DEFUN([AM_SET_AMRWB],
[
# Currently only iLBC in contrib supported
    PLUGINS="${PLUGINS} AMR-WB"
    AMRWB_TARGET="plgamrwb"
    AMRWB_INCLUDE="-I${PWD}/sipXmediaLib/contrib/libamrwb/"
    AMRWB_LIB_ROOT="${PWD}/sipXmediaLib/contrib/libamrwb/"
    AC_SUBST(AMRWB_TARGET)    
    AC_SUBST(AMRWB_INCLUDE)    
    AC_SUBST(AMRWB_LIB_ROOT)

    # amr wideband codec has it's own configure, 
    # so be sure to call it.
    AC_CONFIG_SUBDIRS([sipXmediaLib/contrib/libamrwb/])
])dnl

AC_DEFUN([CHECK_AMR_AMRWB],
[
    amr_enable=false;
    AC_ARG_ENABLE([codec-amr],
                  [AS_HELP_STRING([--enable-codec-amr],
                                  [Enable support for amr codec @<:@default=no@:>@])],
                  [ if test ${enableval} = yes; 
                    then
                           amr_enable=true;
                    else
                       if test ${enableval} = no; then
                           AC_MSG_RESULT(Codec AMR was disabled)
                       else
                           AC_MSG_ERROR(bad value ${enableval} for --enable-codec-amr)
                       fi
                    fi],
                  amr_enable=false)
    if (test "x$amr_enable" = "xtrue"); then 
        AM_SET_AMR
    fi
    AM_CONDITIONAL(AMRNB, [test "x$amr_enable" == "xtrue"])    

    amrwb_enable=false;
    AC_ARG_ENABLE([codec-amrwb],
                  [AS_HELP_STRING([--enable-codec-amrwb],
                                  [Enable support for amrwb codec @<:@default=no@:>@])],
                  [ if test ${enableval} = yes; 
                    then
                           amrwb_enable=true;
                    else
                       if test ${enableval} = no; then
                           AC_MSG_RESULT(Codec AMRWB was disabled)
                       else
                           AC_MSG_ERROR(bad value ${enableval} for --enable-codec-amrwb)
                       fi
                    fi],
                  amrwb_enable=false)
    if (test "x$amrwb_enable" = "xtrue"); then 
        AM_SET_AMRWB
    fi
    AM_CONDITIONAL(AMRWB, [test "x$amrwb_enable" == "xtrue"])    

])dnl


# ============ D O X Y G E N ==================
# Originaly from CppUnit BB_ENABLE_DOXYGEN

AC_DEFUN([ENABLE_DOXYGEN],
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
AC_DEFUN([CHECK_VA_LIST],
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

