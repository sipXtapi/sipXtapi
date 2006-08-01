
# hint: type  "make VARIABLE=XXX" to override any value

# Where downloaded files are kept between builds
LIBSRC=~/libsrc

# Where your system builds RPMS
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')

# /bin/arch may return i686 but RPMs default arch is i386.
# hint: type  "make RPM_TARGET_ARCH=XXX" to override this location
RPM_TARGET_ARCH = i386

# URLs to files pulled down files
RUBY_AUX_RPMS_URL = http://people.redhat.com/dlutter/yum
MOD_CPLUSPLUS_URL = http://umn.dl.sourceforge.net/sourceforge/modcplusplus
JPKG_NONFREE_URL = http://mirrors.dotsrc.org/jpackage/1.6/generic/non-free
CGICC_URL = http://ftp.gnu.org/gnu/cgicc
XERCES_C_URL = http://www.apache.org/dist/xml/xerces-c/source
RUBY_RPM_URL = http://dev.centos.org/centos/4/testing
FC4_RUBY_RPM_URL = http://download.fedora.redhat.com/pub/fedora/linux/core/updates/4
RE4_W3C_URL = http://ftp.redhat.com/pub/redhat/linux/enterprise/4/en/os/i386
