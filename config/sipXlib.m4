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
    SIPXINC=${prefix}/include
    SF_CXX_C_FLAGS="-D__pingtel_on_posix__ -D_linux_ -D_REENTRANT -DCPU=I80486 -march=pentium -fmessage-length=0 -I$SIPXINC $PCRE_CFLAGS"
    SF_CXX_WARNINGS="-Wall -Wformat -Wnested-externs -Wmissing-declarations -Wwrite-strings -Wpointer-arith -Wmissing-prototypes"
    CXXFLAGS="$CXXFLAGS $SF_CXX_C_FLAGS $SF_CXX_WARNINGS"
    CFLAGS="$CFLAGS $SF_CXX_C_FLAGS $SF_CXX_WARNINGS"
    LDFLAGS="$LDFLAGS -lrt -lpthread -lresolv -ldl -lstdc++"
  
    ## location of etc dir is debateable so isolate it here.
    ##
    ## NOTE: These are not expanded (e.g. contains $(prefix)) and only
    ## fit for Makefiles. You can however write a Makefile that transforms
    ## *.in to * with the concrete values. 
    ##
    ##  See sipXconfig/Makefile.am for and example.   
    ##  See autoconf manual 4.7.2 Installation Directory Variables for why it's restricted
    ##
    AC_SUBST(SIPXPBXCONF, [${sysconfdir}/sipxpbx])
    AC_SUBST(SIPXPBXDATA, [${datadir}/sipxpbx])

    ## Used in a number of different project and subjective where this should really go
    ## INSTALL instruction assume default, otherwise safe to change/override
    AC_ARG_VAR(wwwdir, [Web root for apache 2, default is ${prefix}/www. \
                        WARNING: Adjust accordingly when following INSTALL instructions])
    test -z $wwwdir && wwwdir='${prefix}/www'

    AC_ARG_VAR(SIPXPBXUSER, [User sipxchange runs under, default is 'sipxchange'])
    test -z $SIPXPBXUSER && SIPXPBXUSER=sipxchange

    AC_SUBST(SIPXPHONECONF, [${sysconfdir}/sipxphone])
    AC_SUBST(SIPXPHONEDATA, [${datadir}/sipxphone])
    AC_SUBST(SIPXPHONELIB, [${datadir}/sipxphone/lib])
])

## RougeWave
# SFAC_LIB_RW attempts to find the roguewave library and include files by
# looking in /usr/[lib|include], /usr/local/[lib|include], and relative
# paths.
#
# If not found, the configure is aborted.  Otherwise, variables are defined
# for both the INC and LIB paths AND the paths are added to the CFLAGS, 
# CXXFLAGS, LDFLAGS, and LIBS.
AC_DEFUN([SFAC_LIB_RW],
[
    SFAC_ARG_WITH_INCLUDE([rw/rwerr.h],
            [rwtoolsppinc],
            [ --with-rwtoolsppinc=<dir> roguewave toolspp include path ],
            [rwToolsppLib])
    RWTOOLSPPINC=$foundpath
    AC_SUBST(RWTOOLSPPINC)
    CFLAGS="$CFLAGS -I$RWTOOLSPPINC"
    CXXFLAGS="$CXXFLAGS -I$RWTOOLSPPINC"

    SFAC_ARG_WITH_LIB([libtoolspp.la],
            [rwtoolspplib],
            [ --with-rwtoolspplib=<dir> roguewave toolspp library path ],
            [rwToolsppLib])
    RWTOOLSPPLIB=$foundpath
    LIBS="$LIBS $RWTOOLSPPLIB/libtoolspp.la"
    AC_SUBST(RWTOOLSPPLIB)

]) # SFAC_LIB_RW


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
    SIPXPORTINC=$foundpath
    AC_SUBST(SIPXPORTINC)

    if test "$SIPXPORTINC" != "$SIPXINC"
    then
        CFLAGS="$CFLAGS -I$SIPXPORTINC"
        CXXFLAGS="$CXXFLAGS -I$SIPXPORTINC"
    fi

    SFAC_ARG_WITH_LIB([libsipXport.la],
            [sipxportlib],
            [ --with-sipxportlib=<dir> portability library path ],
            [sipXportLib])
    SIPXPORTLIB=$foundpath
    LDFLAGS="$LDFLAGS $PCRE_LDFLAGS -L$SIPXPORTLIB"
    LIBS="$SIPXPORTLIB/libsipXport.la $PCRE_LIBS $LIBS"
    AC_SUBST(SIPXPORTLIB)

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
            [sipxportinc],
            [ --with-sipxtackinc=<dir> sip stack include path ],
            [sipXtackLib])
    SIPXTACKINC=$foundpath
    AC_SUBST(SIPXTACKINC)

    if test "$SIPXTACKINC" != "$SIPXINC"
    then
        CFLAGS="$CFLAGS -I$SIPXTACKINC"
        CXXFLAGS="$CXXFLAGS -I$SIPXTACKINC"
    fi

    SFAC_ARG_WITH_LIB([libsipXtack.la],
            [sipxtacklib],
            [ --with-sipxtacklib=<dir> sip stack library path ],
            [sipXtackLib])
    SIPXTACKLIB=$foundpath
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
    SIPXMEDIAINC=$foundpath
    AC_SUBST(SIPXMEDIAINC)

    if test "$SIPXMEDIAINC" != "$SIPXINC"
    then
        CFLAGS="$CFLAGS -I$SIPXMEDIAINC"
        CXXFLAGS="$CXXFLAGS -I$SIPXMEDIAINC"
    fi
    
    SFAC_ARG_WITH_LIB([libsipXmedia.la],
            [sipxmedialib],
            [ --with-sipxmedialib=<dir> media library path ],
            [sipXmediaLib])
    SIPXMEDIALIB=$foundpath

    # repeated because of circular dependency GIPS and media code
    LIBS="$SIPXMEDIALIB/libsipXmedia.la $LIBS"
]) # SFAC_LIB_MEDIA


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
    SIPXCALLINC=$foundpath
    AC_SUBST(SIPXCALLINC)

    if test "$SIPXCALLINC" != "$SIPXINC"
    then
        CFLAGS="-I$SIPXCALLINC $CFLAGS"
        CXXFLAGS="$CXXFLAGS -I$SIPXCALLINC"
    fi

    SFAC_ARG_WITH_LIB([libsipXcall.la],
            [sipxcalllib],
            [ --with-sipxcalllib=<dir> call processing library path ],
            [sipXcallLib])
    SIPXCALLLIB=$foundpath
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
    SIPXCOMMSERVERINC=$foundpath
    if test "$SIPXCOMMSERVERINC" != "$SIPXINC"
    then
        CFLAGS="$CFLAGS -I$SIPXCOMMSERVERINC"
        CXXFLAGS="$CXXFLAGS -I$SIPXCOMMSERVERINC"
    fi
    AC_SUBST(SIPXCOMMSERVERINC)

    SFAC_ARG_WITH_LIB([libsipXcommserver.la],
            [sipxcommserverlib],
            [ --with-sipxcommserverlib=<dir> call processing library path ],
            [sipXcommserverLib])
    SIPXCOMMSERVERLIB=$foundpath
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
        [ foundpath=$withval ],
        [
        foundrw="no"
        for dir in $withval [$prefix] /usr /usr/local [$abs_srcdir]/../[$4] ; do
            if test -f "$dir/include/[$1]";
            then
                foundpath=$dir/include;
                foundrw="yes";
                break;
            fi;
        done

        if test x_$foundrw != x_yes; then
            AC_MSG_ERROR(not found)
        else
            AC_MSG_RESULT($foundpath)
        fi
        ]
    )

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
        [ foundpath=$withval ],
        [
        foundrw="no"
        for dir in $withval [$prefix]/lib /usr/lib /usr/local/lib [$abs_srcdir]/../[$4]/src ; do
            if test -f "$dir/[$1]";
            then
                foundpath=$dir;
                foundrw="yes";
                break;
            fi;
        done

        if test x_$foundrw != x_yes; then
            AC_MSG_ERROR(Cannot find [$4] library file -- is it compiled?)
        else
            AC_MSG_RESULT($foundpath)
        fi
        ]
    )
]) # SFAC_ARG_WITH_LIB

AC_DEFUN([SFAC_SRCDIR_EXPAND], 
[
    abs_srcdir=`cd $srcdir && pwd`
])
