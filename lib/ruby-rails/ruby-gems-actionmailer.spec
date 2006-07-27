# Generated from actionmailer-1.1.5.gem by gem2spec -*- rpm-spec -*-
%define rbname actionmailer
%define version 1.1.5
%define release 1

Summary: Service layer for easy email delivery and testing.
Name: ruby-gems-%{rbname}

Version: %{version}
Release: %{release}
Group: Development/Ruby
License: Distributable
URL: http://www.rubyonrails.org
Source0: %{rbname}-%{version}.gem
# Make sure the spec template is included in the SRPM
Source1: ruby-gems-%{rbname}.spec.in
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Requires: ruby 
Requires: ruby-gems >= 0.8.11
Requires: ruby-gems-actionpack = 1.11.2
BuildRequires: ruby 
BuildRequires: ruby-gems >= 0.8.11
BuildArch: noarch
Provides: ruby(Actionmailer) = %{version}

%define gemdir /usr/lib/ruby/gems/1.8
%define gembuilddir %{buildroot}%{gemdir}

%description
Makes it trivial to test and deliver emails sent from a single service layer.
%prep
%setup -T -c

%build

%install
%{__rm} -rf %{buildroot}
mkdir -p %{gembuilddir}
gem install --local --install-dir %{gembuilddir} --force --rdoc %{SOURCE0}

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root)
%{gemdir}/gems/actionmailer-1.1.5/rakefile
%{gemdir}/gems/actionmailer-1.1.5/install.rb
%{gemdir}/gems/actionmailer-1.1.5/README
%{gemdir}/gems/actionmailer-1.1.5/CHANGELOG
%{gemdir}/gems/actionmailer-1.1.5/MIT-LICENSE
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/adv_attr_accessor.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/base.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/helpers.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/mail_helper.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/part.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/part_container.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/quoting.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/utils.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/version.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/text/format.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/address.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/attachments.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/base64.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/config.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/encode.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/facade.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/header.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/info.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/loader.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/mail.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/mailbox.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/mbox.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/net.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/obsolete.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/parser.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/port.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/quoting.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/scanner.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/scanner_r.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/stringio.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/tmail.rb
%{gemdir}/gems/actionmailer-1.1.5/lib/action_mailer/vendor/tmail/utils.rb
%{gemdir}/gems/actionmailer-1.1.5/test/mail_helper_test.rb
%{gemdir}/gems/actionmailer-1.1.5/test/mail_render_test.rb
%{gemdir}/gems/actionmailer-1.1.5/test/mail_service_test.rb
%{gemdir}/gems/actionmailer-1.1.5/test/quoting_test.rb
%{gemdir}/gems/actionmailer-1.1.5/test/tmail_test.rb
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/raw_email
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/raw_email10
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/raw_email11
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/raw_email12
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/raw_email2
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/raw_email3
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/raw_email4
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/raw_email5
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/raw_email6
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/raw_email7
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/raw_email8
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/raw_email9
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/helper_mailer/use_helper.rhtml
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/helper_mailer/use_helper_method.rhtml
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/helper_mailer/use_mail_helper.rhtml
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/helper_mailer/use_test_helper.rhtml
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/helpers/test_helper.rb
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/templates/signed_up.rhtml
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/test_mailer/implicitly_multipart_example.text.html.rhtml
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/test_mailer/implicitly_multipart_example.text.plain.rhtml
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/test_mailer/implicitly_multipart_example.text.yaml.rhtml
%{gemdir}/gems/actionmailer-1.1.5/test/fixtures/test_mailer/signed_up.rhtml
%doc %{gemdir}/doc/actionmailer-1.1.5
%{gemdir}/cache/actionmailer-1.1.5.gem
%{gemdir}/specifications/actionmailer-1.1.5.gemspec

%changelog
* Sat Jan 28 2006 David Lutterkort <dlutter@redhat.com> - 1.1.5-1
- Initial build
