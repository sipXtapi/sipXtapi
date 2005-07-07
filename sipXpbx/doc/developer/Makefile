# Makefile for building the Media Server (sipXvxml).
# This file assumes that all of the auxiliary open-source packages have been
# installed in their ordinary (system) locations.

# A list of all the sipX components, in the dependency order that they have
# to be built in.
# For dependency order, see http://www.sipfoundry.org/sipX/index.html
COMPONENTS=sipXportLib sipXtackLib sipXmediaLib sipXcallLib sipXcommserverLib sipXvxml sipXconfig sipXregistry sipXproxy sipXpublisher sipXpbx
# The Subversion repository
# Use https: access so that changes can be checked back in.
REPOSITORY=https://scm.sipfoundry.org/rep
# Branch suffix
BRANCH=branches/2.6

# Environment variables for global configuration
# Example:
# Turn off compiler optimization:
#export CFLAGS=-O0
#export CXXFLAGS=-O0

# Environment variables used by some of the components
export ANT_HOME=/opt/apache-ant-1.6.2
export JAVA_HOME=/usr/java/j2sdk1.4.2_05
export JBOSS_HOME=/opt/JBoss-2.4.10_Tomcat-4.0.6/jboss
export TOMCAT_HOME=/opt/JBoss-2.4.10_Tomcat-4.0.6/catalina

# Environment variable to set the user who will own the installed files.
export SIPXPBXUSER=${USER}

# Define $PREFIX as the --prefix argument to ./configure, if any,
# based on the environment's $INSTALL_PREFIX.
ifeq ($(INSTALL_PREFIX),)
    # No --prefix=xxx, which defaults to /usr/local/sipx
    PREFIX=
else
    PREFIX=--prefix=$(INSTALL_PREFIX)
endif

# Define $SUDO as the prefix to commands that runs any necessary sudo, etc.
SUDO=
#SUDO=sudo

.PHONY: update checkout $(COMPONENTS)

# Update all the components (that are already checked out)
update:
	for d in sipX* ; do echo svn update $$d ; svn update $$d ; done

# Check out all the components
checkout:
	for d in $(COMPONENTS) ; do echo svn checkout --quiet $(REPOSITORY)/$$d/$(BRANCH) $$d ; svn checkout --quiet $(REPOSITORY)/$$d/$(BRANCH) $$d ; done

# Show the status of all the components
status:
	for d in $(COMPONENTS) ; do svn status $$d ; done | \
	grep -Ev '^$$|^Performing status on external item at |^X      '
status-u:
	for d in $(COMPONENTS) ; do svn status -u $$d ; done | \
	grep -Ev '^$$|^Performing status on external item at |^X      '

# Show the version numbers for all the components
showversions:
	for d in sipX* ; do ( cd $$d ; svn info ) ; done | grep -E '^$$|URL:|Revision:'

# Build all the components
build build-check-install: $(COMPONENTS)

# How to build each component

sipXportLib:
	cd sipXportLib ; autoreconf --install --force
	cd sipXportLib ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXportLib ; make
	cd sipXportLib ; make check
	cd sipXportLib ; $(SUDO) make install

sipXtackLib:
	cd sipXtackLib ; autoreconf --install --force
	cd sipXtackLib ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXtackLib ; make
	cd sipXtackLib ; make check
	cd sipXtackLib ; $(SUDO) make install

sipXmediaLib:
	cd sipXmediaLib ; autoreconf --install --force
	cd sipXmediaLib ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXmediaLib ; make
	cd sipXmediaLib ; make check
	cd sipXmediaLib ; $(SUDO) make install

sipXcallLib:
	cd sipXcallLib ; autoreconf --install --force
	cd sipXcallLib ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXcallLib ; make
	cd sipXcallLib ; make check
	cd sipXcallLib ; $(SUDO) make install

sipXcommserverLib:
	cd sipXcommserverLib ; autoreconf --install --force
	cd sipXcommserverLib ; ./configure $(PREFIX)
	cd sipXcommserverLib ; make
	cd sipXcommserverLib ; make check
	cd sipXcommserverLib ; $(SUDO) make install

sipXvxml:
	cd sipXvxml ; autoreconf --install --force
	cd sipXvxml ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXvxml ; make
	cd sipXvxml ; $(SUDO) make install

sipXconfig:
	cd sipXconfig ; autoreconf --install --force
	cd sipXconfig ; ./configure $(PREFIX)
	cd sipXconfig ; make
	cd sipXconfig ; $(SUDO) make install

sipXregistry:
	cd sipXregistry ; autoreconf --install --force
	cd sipXregistry ; ./configure $(PREFIX)
	cd sipXregistry ; make
	cd sipXregistry ; $(SUDO) make install

sipXproxy:
	cd sipXproxy ; autoreconf --install --force
	cd sipXproxy ; ./configure $(PREFIX)
	cd sipXproxy ; make
	cd sipXproxy ; $(SUDO) make install

sipXpublisher:
	cd sipXpublisher ; autoreconf --install --force
	cd sipXpublisher ; ./configure $(PREFIX)
	cd sipXpublisher ; make
	cd sipXpublisher ; $(SUDO) make install

sipXpbx:
	cd sipXpbx ; autoreconf --install --force
	cd sipXpbx ; ./configure $(PREFIX)
	cd sipXpbx ; make
	cd sipXpbx ; $(SUDO) make install

# The rpm --install's install both the binary and the devel RPMs of
# the libraries.
# Where rpmbuild will do its work.
RPMBUILD_TOPDIR = $(shell rpm --eval '%{_topdir}')
rpms:
	cd sipXportLib ; make rpm
	sudo rpm --install -p $(RPMBUILD_TOPDIR)/RPMS/i386/sipxportlib-*.i386.rpm
	cd sipXtackLib ; make rpm
	sudo rpm --install -p $(RPMBUILD_TOPDIR)/RPMS/i386/sipxtacklib-*.i386.rpm
	cd sipXmediaLib ; make rpm
	sudo rpm --install -p $(RPMBUILD_TOPDIR)/RPMS/i386/sipxmedialib-*.i386.rpm
	cd sipXcallLib ; make rpm
	sudo rpm --install -p $(RPMBUILD_TOPDIR)/RPMS/i386/sipxcalllib-*.i386.rpm
	cd sipXcommserverLib ; make rpm
	sudo rpm --install -p $(RPMBUILD_TOPDIR)/RPMS/i386/sipxcommserverlib-*.i386.rpm
	cd sipXvxml ; make rpm
	sudo rpm --install -p $(RPMBUILD_TOPDIR)/RPMS/i386/sipxvxml-*.i386.rpm
	cd sipXconfig ; make rpm
	cd sipXregistry ; make rpm
	cd sipXproxy ; make rpm
	cd sipXpublisher ; make rpm
	cd sipXpbx ; make rpm

# All of the following command groups that edit or modify the configuration
# files are designed to be idempotent, that is, if you repeat them, the
# second and later executions have no additional effect.

# Comment out automatically running the config server by the watchdog.
WD=/usr/local/sipx/etc/sipxpbx/WatchDog.xml
no-config-server:
	mv $(WD) $(WD).tmp
	sed <$(WD).tmp >$(WD) \
	    -e '/^[ 	]*<process name="RMIRegistry"/,/<\/process>/s/\(.*\)/<!-- \1 -->/' \
	    -e '/^[ 	]*<process name="ProfilePublisher"/,/<\/process>/s/\(.*\)/<!-- \1 -->/' \
	    -e '/^[ 	]*<process name="ProfileWriter"/,/<\/process>/s/\(.*\)/<!-- \1 -->/' \
	    -e '/^[ 	]*<process name="ProfileGenerator"/,/<\/process>/s/\(.*\)/<!-- \1 -->/'
	rm $(WD).tmp

# Comment out automatically running the media server by the watchdog.
no-media-server:
	mv $(WD) $(WD).tmp
	sed <$(WD).tmp >$(WD) \
	    -e '/^[ 	]*<process name="MediaServer"/,/<\/process>/s/\(.*\)/<!-- \1 -->/'
	rm $(WD).tmp

# Install test certificate files so Apache can do SSL.
install-test-certificate:
	mkdir -p /usr/local/sipx/etc/sipxpbx/ssl
	cp ~/{ssl.crt,ssl.key} /usr/local/sipx/etc/sipxpbx/ssl

# Install test configuration information from the test-config directory.
install-test-config:
	cp ./sipXpbx/doc/developer/test-config/var=sipxdata=sipdb/* \
		/usr/local/sipx/var/sipxdata/sipdb

# Edit config.defs to set SIPXCHANGE_DOMAIN_NAME=`hostname -f`
CONFIG_DEFS=/usr/local/sipx/etc/sipxpbx/config.defs
install-host-name:
	mv $(CONFIG_DEFS) $(CONFIG_DEFS).tmp
	sed <$(CONFIG_DEFS).tmp >$(CONFIG_DEFS) \
	    -e '/^ *SIPXCHANGE_DOMAIN_NAME *=/s/=.*$$/=`hostname -f`/'
	rm $(CONFIG_DEFS).tmp

# Build and test the software, but do not install it.
# (Which may not work if the installed libraries are too different from the
# current code, since the links will be done against the installed libraries,
# not the ones built during this process!)
# (This should be factored better, as it duplicates the above entries.)
build-check:
	cd sipXportLib ; autoreconf --install --force
	cd sipXportLib ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXportLib ; make
	cd sipXportLib ; make check
	cd sipXtackLib ; autoreconf --install --force
	cd sipXtackLib ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXtackLib ; make
	cd sipXtackLib ; make check
	cd sipXmediaLib ; autoreconf --install --force
	cd sipXmediaLib ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXmediaLib ; make
	cd sipXmediaLib ; make check
	cd sipXcallLib ; autoreconf --install --force
	cd sipXcallLib ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXcallLib ; make
	cd sipXcallLib ; make check
	cd sipXcommserverLib ; autoreconf --install --force
	cd sipXcommserverLib ; ./configure $(PREFIX)
	cd sipXcommserverLib ; make
	cd sipXcommserverLib ; make check
	cd sipXvxml ; autoreconf --install --force
	cd sipXvxml ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXvxml ; make
	cd sipXconfig ; autoreconf --install --force
	cd sipXconfig ; ./configure $(PREFIX)
	cd sipXconfig ; make
	cd sipXregistry ; autoreconf --install --force
	cd sipXregistry ; ./configure $(PREFIX)
	cd sipXregistry ; make
	cd sipXproxy ; autoreconf --install --force
	cd sipXproxy ; ./configure $(PREFIX)
	cd sipXproxy ; make
	cd sipXpublisher ; autoreconf --install --force
	cd sipXpublisher ; ./configure $(PREFIX)
	cd sipXpublisher ; make
	cd sipXpbx ; autoreconf --install --force
	cd sipXpbx ; ./configure $(PREFIX)
	cd sipXpbx ; make

# Build and install the software, but do not test it.
# (This should be factored better, as it duplicates the above entries.)
build-install:
	cd sipXportLib ; autoreconf --install --force
	cd sipXportLib ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXportLib ; make
	cd sipXportLib ; $(SUDO) make install
	cd sipXtackLib ; autoreconf --install --force
	cd sipXtackLib ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXtackLib ; make
	cd sipXtackLib ; $(SUDO) make install
	cd sipXmediaLib ; autoreconf --install --force
	cd sipXmediaLib ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXmediaLib ; make
	cd sipXmediaLib ; $(SUDO) make install
	cd sipXcallLib ; autoreconf --install --force
	cd sipXcallLib ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXcallLib ; make
	cd sipXcallLib ; $(SUDO) make install
	cd sipXcommserverLib ; autoreconf --install --force
	cd sipXcommserverLib ; ./configure $(PREFIX)
	cd sipXcommserverLib ; make
	cd sipXcommserverLib ; $(SUDO) make install
	cd sipXvxml ; autoreconf --install --force
	cd sipXvxml ; ./configure $(PREFIX) --with-pcre_includedir=/usr/include/pcre --with-pcre_libdir=/usr/lib
	cd sipXvxml ; make
	cd sipXvxml ; $(SUDO) make install
	cd sipXconfig ; autoreconf --install --force
	cd sipXconfig ; ./configure $(PREFIX)
	cd sipXconfig ; make
	cd sipXconfig ; $(SUDO) make install
	cd sipXregistry ; autoreconf --install --force
	cd sipXregistry ; ./configure $(PREFIX)
	cd sipXregistry ; make
	cd sipXregistry ; $(SUDO) make install
	cd sipXproxy ; autoreconf --install --force
	cd sipXproxy ; ./configure $(PREFIX)
	cd sipXproxy ; make
	cd sipXproxy ; $(SUDO) make install
	cd sipXpublisher ; autoreconf --install --force
	cd sipXpublisher ; ./configure $(PREFIX)
	cd sipXpublisher ; make
	cd sipXpublisher ; $(SUDO) make install
	cd sipXpbx ; autoreconf --install --force
	cd sipXpbx ; ./configure $(PREFIX)
	cd sipXpbx ; make
	cd sipXpbx ; $(SUDO) make install

make-install:
	cd sipXportLib ; make
	cd sipXportLib ; $(SUDO) make install
	cd sipXtackLib ; make
	cd sipXtackLib ; $(SUDO) make install
	cd sipXmediaLib ; make
	cd sipXmediaLib ; $(SUDO) make install
	cd sipXcallLib ; make
	cd sipXcallLib ; $(SUDO) make install
	cd sipXcommserverLib ; make
	cd sipXcommserverLib ; $(SUDO) make install
	cd sipXvxml ; make
	cd sipXvxml ; $(SUDO) make install
	cd sipXconfig ; make
	cd sipXconfig ; $(SUDO) make install
	cd sipXregistry ; make
	cd sipXregistry ; $(SUDO) make install
	cd sipXproxy ; make
	cd sipXproxy ; $(SUDO) make install
	cd sipXpublisher ; make
	cd sipXpublisher ; $(SUDO) make install
	cd sipXpbx ; make
	cd sipXpbx ; $(SUDO) make install

install:
	cd sipXportLib ; $(SUDO) make install
	cd sipXtackLib ; $(SUDO) make install
	cd sipXmediaLib ; $(SUDO) make install
	cd sipXcallLib ; $(SUDO) make install
	cd sipXcommserverLib ; $(SUDO) make install
	cd sipXvxml ; $(SUDO) make install
	cd sipXconfig ; $(SUDO) make install
	cd sipXregistry ; $(SUDO) make install
	cd sipXproxy ; $(SUDO) make install
	cd sipXpublisher ; $(SUDO) make install
	cd sipXpbx ; $(SUDO) make install

# Check that the rules for tabs have not been violated
# The complex 'find' selects the files within which we want to enforce the 
# ban on tabs.  The '-prune -false -o -type f' construction prevents
# recursion into directories of specified names, but also restricts the
# output of the find to ordinary files.  The long list of -name's before
# it is the list of directory names that will not be examined.
# The next list of -name's specify which files will not be examined.
# The first xargs ... grep selects files containing at least one tab.
# (The construction "$$'\t'" is a way to specify a tab (in bash) without
# using one.)  The second xargs ... grep removes files with a SpeechWorks
# copyright, as they are part of VXI.  The final grep is to produce a
# correct exit code, as grep --files-without-match doesn't exit 0/1
# based on whether any file names are output or not.
enforce-tabs:
	! find sipXregistry sipXvxml \
		\( \( -name .svn -o \
			-name jsi -o \
			-name VXI -o \
			-name js -o \
			-name meta -o \
			-name doc -o \
			-name config -o \
			-name autom4te.cache \) -prune -false -o -type f \) \
		! \( -name 'Makefile*' -o \
			-name '*.o' -o \
			-name '*.lo' -o \
			-name '*.so' -o \
			-name '*.so.[0-9]*' -o \
			-name '*.a' -o \
			-name '*.class' -o \
			-name '*.jar' -o \
			-name '*.gif' -o \
			-name COPYING -o \
			-name LICENSE -o \
			-name aclocal.m4 -o \
			-name configure -o \
			-name config.log -o \
			-name config.status -o \
			-name sipXvxml -o \
			-name sipregistrar -o \
			-name libtool \) \
		-print0 | \
	xargs --null grep --null --files-with-matches $$'\t' | \
	xargs --null grep --files-without-match SpeechWorks | \
	grep '.'

# Use a trick to make sure that "NO$(NULL)NE" does not appear in this makefile.
NULL=

# Search for occurrences of NO$(NULL)NE in the makefiles, which indicate an
# incorrect substitution of a configure variable (usually ${prefix}
# for which the user did not give an option to ./configure.
search-NO$(NULL)NE:
	! grep -n NO$(NULL)NE \
		$$( find sipX* -name '.svn' -prune -false -o -name Makefile ) | \
	grep '.'

# Search for un-substituted variables that should have been substituted.
# Currently, only checks for @... variables in the install directory.
search-unsubstituted:
	! grep -rE --binary-files=without-match '@[A-Za-z]+@' /usr/local/sipx | \
	grep '.'

# Print the svnversion info.
svnversion:
	for d in $(COMPONENTS) ; do echo -n "$d: " ; svnversion $d ; done
