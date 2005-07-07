##
## Libs from SipFoundry
##

## Common C and C++ flags for pingtel related source
AC_DEFUN([SFAC_INIT_FLAGS],
[
    AC_REQUIRE([CHECK_PCRE])

    ## TODO Remove cpu specifics and use make variables setup for this
    ##
    ## NOTES:
    ##   -D__pingtel_on_posix__   - really used for linux v.s. other
    ##   -D_REENTRANT             - rougewave ?
    ##   -fmessage-length=0       - ?
    ##
    AC_SUBST(SIPX_INCDIR, [${includedir}])
    AC_SUBST(SIPX_LIBDIR, [${libdir}])

    SF_CXX_C_FLAGS="-D__pingtel_on_posix__ -D_linux_ -D_REENTRANT -D_FILE_OFFSET_BITS=64 -fmessage-length=0 -DSIPX_VERSION=\\\"${PACKAGE_VERSION}\\\" -I$SIPX_INCDIR $GLIB_CFLAGS $PCRE_CFLAGS"
    SF_CXX_WARNINGS="-Wall -Wformat -Wwrite-strings -Wpointer-arith"
    CXXFLAGS="$CXXFLAGS $SF_CXX_C_FLAGS $SF_CXX_WARNINGS"
    CFLAGS="$CFLAGS $SF_CXX_C_FLAGS $SF_CXX_WARNINGS -Wnested-externs -Wmissing-declarations -Wmissing-prototypes"
    LDFLAGS="$LDFLAGS -L$SIPX_LIBDIR $GLIB_LIBS -lrt -lpthread -lresolv -ldl -lstdc++"
  

    ## NOTE: These are not expanded (e.g. contain $(prefix)) and are only
    ## fit for Makefiles. You can however write a Makefile that transforms
    ## *.in to * with the concrete values. 
    ##
    ##  See sipXconfig/Makefile.am for and example.   
    ##  See autoconf manual 4.7.2 Installation Directory Variables for why it's restricted
    ##
    AC_SUBST(SIPX_BINDIR,  [${bindir}])
    AC_SUBST(SIPX_CONFDIR, [${sysconfdir}/sipxpbx])
    AC_SUBST(SIPX_DATADIR, [${datadir}/sipxpbx])
    AC_SUBST(SIPX_LOGDIR,  [${localstatedir}/log/sipxpbx])
    AC_SUBST(SIPX_RUNDIR,  [${localstatedir}/run/sipxpbx])
    AC_SUBST(SIPX_TMPDIR,  [${localstatedir}/tmp])
    AC_SUBST(SIPX_DBDIR,   [${localstatedir}/sipxdata/sipdb])

    # temporary - see http://track.sipfoundry.org/browse/XPB-33
    AC_SUBST(SIPX_VXMLDATADIR,[${localstatedir}/sipxdata/mediaserver/data])

    # temporary - see http://track.sipfoundry.org/browse/XPB-93
    AC_SUBST(SIPX_CONFIGPHONEDIR, [${localstatedir}/sipxdata/configserver/phone])

    ## Used in a number of different project and subjective where this should really go
    ## INSTALL instruction assume default, otherwise safe to change/override
    AC_ARG_VAR(wwwdir, [Web root for web content, default is ${datadir}/www. \
                        WARNING: Adjust accordingly when following INSTALL instructions])
    test -z $wwwdir && wwwdir='${datadir}/www'

    AC_ARG_VAR(SIPXPBXUSER, [The user that sipXpbx runs under, default is 'sipxchange'])
    test -z $SIPXPBXUSER && SIPXPBXUSER=sipxchange

    AC_SUBST(SIPXPHONECONF, [${sysconfdir}/sipxphone])
    AC_SUBST(SIPXPHONEDATA, [${datadir}/sipxphone])
    AC_SUBST(SIPXPHONELIB, [${datadir}/sipxphone/lib])

    AC_ARG_ENABLE(rpmbuild, 
      [  --enable-rpmbuild       Build an rpm],
      enable_rpmbuild=yes )

    AC_CONFIG_FILES([config/sipX-config], [chmod +x config/sipX-config])
])

## Check to see that we are using the minimum required version of automake
AC_DEFUN([SFAC_AUTOMAKE_VERSION],[
   AC_MSG_CHECKING(for automake version >= $1)
   sf_am_version=`automake --version | head -n 1 | awk '/^automake/ {print $NF}'`
   AX_COMPARE_VERSION( [$1], [le], [$sf_am_version], AC_MSG_RESULT( $sf_am_version is ok), AC_MSG_ERROR( found $sf_am_version - you must upgrade automake ))
])

## sipXportLib 
# SFAC_LIB_PORT attempts to find the sf portability library and include
# files by looking in /usr/[lib|include], /usr/local/[lib|include], and
# relative paths.
#
# If not found, the configure is aborted.  Otherwise, variables are defined
# for both the INC and LIB paths AND the paths are added to the CFLAGS, 
# CXXFLAGS, LDFLAGS, and LIBS.
AC_DEFUN([SFAC_LIB_PORT],
[
    AC_REQUIRE([SFAC_INIT_FLAGS])
    SFAC_ARG_WITH_INCLUDE([os/OsDefs.h],
            [sipxportinc],
            [ --with-sipxportinc=<dir> portability include path ],
            [sipXportLib])

    if test x_$foundpath != x_; then
        AC_MSG_RESULT($foundpath)
    else
        AC_MSG_ERROR('os/OsDefs.h' not found)
    fi
    SIPXPORTINC=$foundpath
    AC_SUBST(SIPXPORTINC)

    if test "$SIPXPORTINC" != "$SIPX_INCDIR"
    then
        CFLAGS="$CFLAGS -I$SIPXPORTINC"
        CXXFLAGS="$CXXFLAGS -I$SIPXPORTINC"
    fi

    SFAC_ARG_WITH_LIB([libsipXport.la],
            [sipxportlib],
            [ --with-sipxportlib=<dir> portability library path ],
            [sipXportLib])

    if test x_$foundpath != x_; then
        AC_MSG_RESULT($foundpath)
    else
        AC_MSG_ERROR('libsipXport.la' not found)
    fi

    LDFLAGS="$LDFLAGS $PCRE_LDFLAGS -L$SIPXPORTLIB/test/sipxunit"

    SIPXPORTLIB=$foundpath
    if test "$SIPXPORTLIB" != "$SIPX_LIBDIR"
    then
        LDFLAGS="$LDFLAGS -L$SIPXPORTLIB"
    fi

    LIBS="$SIPXPORTLIB/libsipXport.la $PCRE_LIBS $LIBS"
    AC_SUBST(SIPXPORTLIB)

    # sipXunit unitesting support
    AC_SUBST(SIPXUNIT_SRCDIR, [${includedir}])
    AC_SUBST(SIPXUNIT_LDADD, ["-lcppunit -lsipXunit"])
]) # SFAC_LIB_PORT


## sipXtackLib 
# SFAC_LIB_STACK attempts to find the sf portability library and include
# files by looking in /usr/[lib|include], /usr/local/[lib|include], and
# relative paths.
#
# If not found, the configure is aborted.  Otherwise, variables are defined
# for both the INC and LIB paths AND the paths are added to the CFLAGS, 
# CXXFLAGS, LDFLAGS, and LIBS.
AC_DEFUN([SFAC_LIB_STACK],
[
    AC_REQUIRE([SFAC_LIB_PORT])
    SFAC_ARG_WITH_INCLUDE([net/SipUserAgent.h],
            [sipxtackinc],
            [ --with-sipxtackinc=<dir> sip stack include path ],
            [sipXtackLib])

    if test x_$foundpath != x_; then
        AC_MSG_RESULT($foundpath)
    else
        AC_MSG_ERROR('net/SipUserAgent.h' not found)
    fi
    SIPXTACKINC=$foundpath
    AC_SUBST(SIPXTACKINC)

    if test "$SIPXTACKINC" != "$SIPX_INCDIR"
    then
        CFLAGS="$CFLAGS -I$SIPXTACKINC"
        CXXFLAGS="$CXXFLAGS -I$SIPXTACKINC"
    fi

    SFAC_ARG_WITH_LIB([libsipXtack.la],
            [sipxtacklib],
            [ --with-sipxtacklib=<dir> sip stack library path ],
            [sipXtackLib])

    if test x_$foundpath != x_; then
        AC_MSG_RESULT($foundpath)
    else
        AC_MSG_ERROR('libsipXtack.la' not found)
    fi

    SIPXTACKLIB=$foundpath
    if test "$SIPXTACKLIB" != "$SIPX_LIBDIR"
    then
        LDFLAGS="$LDFLAGS -L$SIPXTACKLIB"
    fi
    LIBS="$SIPXTACKLIB/libsipXtack.la $LIBS"
]) # SFAC_LIB_STACK


## sipXmediaLib 
# SFAC_LIB_MEDIA attempts to find the sf media library and include
# files by looking in /usr/[lib|include], /usr/local/[lib|include], and
# relative paths.
#
# If not found, the configure is aborted.  Otherwise, variables are defined
# for both the INC and LIB paths AND the paths are added to the CFLAGS, 
# CXXFLAGS, LDFLAGS, and LIBS.
AC_DEFUN([SFAC_LIB_MEDIA],
[
    AC_REQUIRE([SFAC_LIB_PORT])
    SFAC_ARG_WITH_INCLUDE([mp/MpMediaTask.h],
            [sipxmediainc],
            [ --with-sipxmediainc=<dir> media library include path ],
            [sipXmediaLib])

    if test x_$foundpath != x_; then
        AC_MSG_RESULT($foundpath)
    else
        AC_MSG_ERROR('mp/MpMediaTask.h' not found)
    fi
    SIPXMEDIAINC=$foundpath
    AC_SUBST(SIPXMEDIAINC)

    if test "$SIPXMEDIAINC" != "$SIPX_INCDIR"
    then
        CFLAGS="$CFLAGS -I$SIPXMEDIAINC"
        CXXFLAGS="$CXXFLAGS -I$SIPXMEDIAINC"
    fi
    
    SFAC_ARG_WITH_LIB([libsipXmedia.la],
            [sipxmedialib],
            [ --with-sipxmedialib=<dir> media library path ],
            [sipXmediaLib])

    if test x_$foundpath != x_; then
        AC_MSG_RESULT($foundpath)
    else
        AC_MSG_ERROR('libsipXmedia.la' not found)
    fi
    SIPXMEDIALIB=$foundpath
    if test "$SIPXMEDIALIB" != "$SIPX_LIBDIR"
    then
        LDFLAGS="$LDFLAGS -L$SIPXMEDIALIB"
    fi

    # repeated because of circular dependency GIPS and media code
    LIBS="$SIPXMEDIALIB/libsipXmedia.la $LIBS"
]) # SFAC_LIB_MEDIA

## Optionally compile in the GIPS library in the media subsystem
# (sipXmediaLib project) and executables that link it in
# Conditionally use the GIPS audio libraries
AC_DEFUN([CHECK_GIPS],
[
   AC_ARG_WITH(gips,
      [  --with-gips       Compile the media subsystem with the GIPS audio libraries
],
      compile_with_gips=yes)

   AC_MSG_CHECKING(if compile with gips)

   if test x$compile_with_gips = xyes
   then
      AC_MSG_RESULT(gips)
      
      SFAC_SRCDIR_EXPAND

      AC_MSG_CHECKING(for gips includes)
      # Define HAVE_GIPS for c pre-processor
      GIPS_CPPFLAGS=-DHAVE_GIPS
      if test -e $withval/include/GIPS/Vendor_gips_typedefs.h
      then
         gips_dir=$withval
      elif test -e $abs_srcdir/../gips/include/GIPS/Vendor_gips_typedefs.h
      then
         gips_dir=$abs_srcdir/../gips
      else
         AC_MSG_ERROR(GIPS/Vendor_gips_typedefs.h not found)
      fi

      AC_MSG_RESULT($gips_dir)

      # Add GIPS include path
      GIPSINC=$gips_dir/include
      # Add GIPS objects to be linked in
      GIPS_OBJS=$gips_dir/GIPS_SuSE_i386.a
      CPPFLAGS="$CPPFLAGS $GIPS_CPPFLAGS -I$GIPSINC"
      # GIPS needs to be at the end of the list
      #LIBS="$LIBS $GIPS_OBJS"
   else
      AC_MSG_RESULT(not compiling in gips)
   fi

   AC_SUBST(GIPSINC)
   AC_SUBST(GIPS_OBJS)
   AC_SUBST(GIPS_CPPFLAGS)
]) # CHECK_GIPS


## sipXcallLib
# SFAC_LIB_CALL attempts to find the sf call processing library and include
# files by looking in /usr/[lib|include], /usr/local/[lib|include], and
# relative paths.
#
# If not found, the configure is aborted.  Otherwise, variables are defined
# for both the INC and LIB paths AND the paths are added to the CFLAGS,
# CXXFLAGS, LDFLAGS, and LIBS.
AC_DEFUN([SFAC_LIB_CALL],
[
    AC_REQUIRE([SFAC_LIB_STACK])
    SFAC_ARG_WITH_INCLUDE([cp/CallManager.h],
            [sipxcallinc],
            [ --with-sipxcallinc=<dir> call processing library include path ],
            [sipXcallLib])

    if test x_$foundpath != x_; then
        AC_MSG_RESULT($foundpath)
    else
        AC_MSG_ERROR('cp/CallManager.h' not found)
    fi
    SIPXCALLINC=$foundpath
    AC_SUBST(SIPXCALLINC)

    if test "$SIPXCALLINC" != "$SIPX_INCDIR"
    then
        CFLAGS="-I$SIPXCALLINC $CFLAGS"
        CXXFLAGS="$CXXFLAGS -I$SIPXCALLINC"
    fi

    SFAC_ARG_WITH_LIB([libsipXcall.la],
            [sipxcalllib],
            [ --with-sipxcalllib=<dir> call processing library path ],
            [sipXcallLib])

    if test x_$foundpath != x_; then
        AC_MSG_RESULT($foundpath)
    else
        AC_MSG_ERROR('libsipXcall.la' not found)
    fi
    SIPXCALLLIB=$foundpath
    if test "$SIPXCALLLIB" != "$SIPX_LIBDIR"
    then
        LDFLAGS="$LDFLAGS -L$SIPXCALLLIB"
    fi

    LIBS="$SIPXCALLLIB/libsipXcall.la $LIBS"
    AC_SUBST(SIPXCALLLIB)

]) # SFAC_LIB_CALL


## sipXcommserverLib

# SFAC_LIB_CALL attempts to find the sf communication server library and 
# include files by looking in /usr/[lib|include], /usr/local/[lib|include], 
# and relative paths.
#
# If not found, the configure is aborted.  Otherwise, variables are defined
# for both the INC and LIB paths AND the paths are added to the CFLAGS,
# CXXFLAGS, LDFLAGS, and LIBS.
AC_DEFUN([SFAC_LIB_COMMSERVER],
[
    AC_REQUIRE([SFAC_LIB_STACK])
    SFAC_ARG_WITH_INCLUDE([sipdb/SIPDBManager.h],
            [sipxcallinc],
            [ --with-sipxcommserverinc=<dir> call processing library include path ],
            [sipXcommserverLib])

    if test x_$foundpath != x_; then
        AC_MSG_RESULT($foundpath)
    else
        AC_MSG_ERROR('sipdb/SIPDBManager.h' not found)
    fi
    SIPXCOMMSERVERINC=$foundpath
    if test "$SIPXCOMMSERVERINC" != "$SIPX_INCDIR"
    then
        CFLAGS="$CFLAGS -I$SIPXCOMMSERVERINC"
        CXXFLAGS="$CXXFLAGS -I$SIPXCOMMSERVERINC"
    fi
    AC_SUBST(SIPXCOMMSERVERINC)

    SFAC_ARG_WITH_LIB([libsipXcommserver.la],
            [sipxcommserverlib],
            [ --with-sipxcommserverlib=<dir> call processing library path ],
            [sipXcommserverLib])

    if test x_$foundpath != x_; then
        AC_MSG_RESULT($foundpath)
    else
        AC_MSG_ERROR('libsipXcommserver.la' not found)
    fi
    SIPXCOMMSERVERLIB=$foundpath
    if test "$SIPXCOMMSERVERLIB" != "$SIPX_LIBDIR"
    then
        LDFLAGS="$LDFLAGS -L$SIPXCOMMSERVERLIB"
    fi

    LIBS="$SIPXCOMMSERVERLIB/libsipXcommserver.la $LIBS"
    AC_SUBST(SIPXCOMMSERVERLIB)

]) # SFAC_LIB_COMMSERVER



##  Generic find of an include
#   Fed from AC_DEFUN([SFAC_INCLUDE_{module name here}],
#
# $1 - sample include file
# $2 - variable name (for overridding with --with-$2
# $3 - help text
# $4 - directory name (assumed parallel with this script)
AC_DEFUN([SFAC_ARG_WITH_INCLUDE],
[
    SFAC_SRCDIR_EXPAND()

    AC_MSG_CHECKING(for [$4] includes)
    AC_ARG_WITH( [$2],
        [ [$3] ],
        [ include_path=$withval ],
        [ include_path="$includedir $prefix/include /usr/include /usr/local/include [$abs_srcdir]/../[$4]/include" ]
    )

    for dir in $include_path ; do
        if test -f "$dir/[$1]";
        then
            foundpath=$dir;
            break;
        fi;
    done
    if test x_$foundpath = x_; then
       AC_MSG_ERROR("'$1' not found; searched $include_path")
    fi
        

]) # SFAC_ARG_WITH_INCLUDE


##  Generic find of a library
#   Fed from AC_DEFUN([SFAC_LIB_{module name here}],
#
# $1 - sample lib file
# $2 - variable name (for overridding with --with-$2
# $3 - help text
# $4 - directory name (assumed parallel with this script)
AC_DEFUN([SFAC_ARG_WITH_LIB],
[
    SFAC_SRCDIR_EXPAND()

    AC_MSG_CHECKING(for [$4] libraries)
    AC_ARG_WITH( [$2],
        [ [$3] ],
        [ lib_path=$withval ],
        [ lib_path="$libdir $prefix/lib /usr/lib /usr/local/lib `pwd`/../[$4]/src" ]
    )
    foundpath=""
    for dir in $lib_path ; do
        if test -f "$dir/[$1]";
        then
            foundpath=$dir;
            break;
        fi;
    done
    if test x_$foundpath = x_; then
       AC_MSG_ERROR("'$1' not found; searched $lib_path")
    fi
]) # SFAC_ARG_WITH_LIB

AC_DEFUN([SFAC_SRCDIR_EXPAND], 
[
    abs_srcdir=`cd $srcdir && pwd`
])
