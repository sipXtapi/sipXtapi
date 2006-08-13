# Place to store RPM output files
AC_DEFUN([SFAC_DIST_DIR],
[
  AC_ARG_WITH([distdir],
    AC_HELP_STRING([--with-distdir=directory], 
      [Directory to output distribution output files like tarballs, srpms and rpms, default is $(top_builddir)/dist]),
    [DIST_DIR=${withval}],[DIST_DIR=${ac_top_builddir}/dist])

  AC_SUBST([RPMBUILD_TOPDIR], [`rpm --eval '%{_topdir}'`])
  AC_SUBST([RPM_TARGET_ARCH], [`rpm --eval '%{_target_cpu}'`])

  mkdir -p "$DIST_DIR" 2>/dev/null
  DIST_DIR=`cd "$DIST_DIR"; pwd`

  # all distro tarballs
  DEST_SRC="${DIST_DIR}/SRC"
  mkdir "${DEST_SRC}"  2>/dev/null
  AC_SUBST([DEST_SRC])

  AC_ARG_VAR([LIBSRC], [Where downloaded files are kept between builds, default ~/libsrc])
  test -z $LIBSRC && LIBSRC=~/libsrc

  # RPM based distros

  DEST_RPM="${DIST_DIR}/RPM"
  mkdir "${DEST_RPM}" 2>/dev/null
  AC_SUBST([DEST_RPM])

  DEST_SRPM="${DIST_DIR}/SRPM"
  mkdir "${DEST_SRPM}"  2>/dev/null
  AC_SUBST([DEST_SRPM])
])


AC_DEFUN([SFAC_DOWNLOAD_DEPENDENCIES],
[  
  # URLs to files pulled down files
  AC_SUBST(RUBY_AUX_RPMS_URL, http://people.redhat.com/dlutter/yum)
  AC_SUBST(MOD_CPLUSPLUS_URL, http://umn.dl.sourceforge.net/sourceforge/modcplusplus)
  AC_SUBST(JPKG_NONFREE_URL, http://mirrors.dotsrc.org/jpackage/1.6/generic/non-free)
  AC_SUBST(CGICC_URL, http://ftp.gnu.org/gnu/cgicc)
  AC_SUBST(XERCES_C_URL, http://www.apache.org/dist/xml/xerces-c/source)
  AC_SUBST(RUBY_RPM_URL, http://dev.centos.org/centos/4/testing)
  AC_SUBST(FC4_RUBY_RPM_URL, http://download.fedora.redhat.com/pub/fedora/linux/core/updates/4)
  AC_SUBST(W3C_URL, http://ftp.redhat.com/pub/redhat/linux/enterprise/4/en/os/i386)
  AC_SUBST(PCRE_URL, http://umn.dl.sourceforge.net/sourceforge/pcre)
  AC_SUBST(DOWNLOAD_FILE, ${srcdir}/config/download-file)
])
