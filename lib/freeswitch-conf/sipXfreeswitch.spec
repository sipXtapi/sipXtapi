Name:     sipxconfig-freeswitch
Version:  1.0
Release:  1

Summary:  sipX conferencing solution based on FreeSWITCH
License:  LGPL
Group:    Productivity/Telephony/SIP/Servers
URL:      http://www.voiceworks.pl/sipX
Vendor:   VoiceWorks Sp. z o.o.
Packager: Pawel Pierscionek <pawel@voiceworks.pl>
Source:   %name.tar.gz
Requires: sipxconfig >= 3.7
Requires: freeswitch = 1.0-snap4sipx
BuildRoot: %{_tmppath}/%name-%version-root

%description
Package contains files needed to enable FreeSWITCH based conferencing in sipX.

%prep
rm -rf $RPM_BUILD_ROOT
%setup -q -n %name

%build
mkdir $RPM_BUILD_ROOT

%install
cp -rp * $RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
/

%post                                                                      
cat  /etc/sipxpbx/resourcemap.xml | sed -e 's/etc\/sipxpbx\/conferences.xml/usr\/local\/freeswitch\/conf\/default_context.xml/' > /var/tmp/tmp_resmap.xml
cat /var/tmp/tmp_resmap.xml > /etc/sipxpbx/resourcemap.xml
rm -f /var/tmp/tmp_resmap.xml
/usr/bin/freeswitch.sh --configtest
