Name:     freeswitch
Version:  1.0
Release:  snap4sipx

Summary:  OSS telephony application
License:  MPL
Group:    Productivity/Telephony/SIP/Servers
URL:      http://www.freeswitch.org/
Vendor:   FreeSwitch
Packager: Pawel Pierscionek <pawel@voiceworks.pl>
Source:   %name-snapshot.tar.gz
Patch:    freeswitch_conf.patch
Requires: sipxconfig >= 3.7

%description
FreeSWITCH is an open source telephony application written in C, 
built from the ground up and designed to take advantage of 
as many existing software libraries as possible. FreeSWITCH makes 
it possible to build an open source PBX system or an open source 
voip switching platform as well as unite various technologies 
such as SIP H.323, IAX2, LDAP, Zeroconf, XMPP / Jingle etc.  
FreeSWITCH can also be used to interface with other open source 
PBX systems such as OpenPBX, Bayonne, YATE or Asterisk

%prep
%setup -q -n freeswitch_snapshot
%patch

%build
autoreconf -if
CFLAGS="$RPM_OPT_FLAGS" ./configure -C 
make

%install
make installall

%clean

%files
/usr/local/freeswitch

%post
