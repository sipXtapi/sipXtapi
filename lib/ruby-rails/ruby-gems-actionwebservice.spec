# Generated from actionwebservice-1.0.0.gem by gem2spec -*- rpm-spec -*-
%define rbname actionwebservice
%define version 1.0.0
%define release 1

Summary: Web service support for Action Pack.
Name: ruby-gems-%{rbname}

Version: %{version}
Release: %{release}
Group: Development/Ruby
License: MIT
URL: http://www.rubyonrails.org
Source0: %{rbname}-%{version}.gem
# Make sure the spec template is included in the SRPM
Source1: ruby-gems-%{rbname}.spec.in
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Requires: ruby 
Requires: ruby-gems >= 0.8.11
Requires: ruby-gems-actionpack = 1.11.2
Requires: ruby-gems-activerecord = 1.13.2
BuildRequires: ruby 
BuildRequires: ruby-gems >= 0.8.11
BuildArch: noarch
Provides: ruby(Actionwebservice) = %{version}

%define gemdir /usr/lib/ruby/gems/1.8
%define gembuilddir %{buildroot}%{gemdir}

%description
Adds WSDL/SOAP and XML-RPC web service support to Action Pack
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
%{gemdir}/gems/actionwebservice-1.0.0/Rakefile
%{gemdir}/gems/actionwebservice-1.0.0/setup.rb
%{gemdir}/gems/actionwebservice-1.0.0/README
%{gemdir}/gems/actionwebservice-1.0.0/TODO
%{gemdir}/gems/actionwebservice-1.0.0/CHANGELOG
%{gemdir}/gems/actionwebservice-1.0.0/MIT-LICENSE
%{gemdir}/gems/actionwebservice-1.0.0/examples/googlesearch/README
%{gemdir}/gems/actionwebservice-1.0.0/examples/googlesearch/autoloading/google_search_api.rb
%{gemdir}/gems/actionwebservice-1.0.0/examples/googlesearch/autoloading/google_search_controller.rb
%{gemdir}/gems/actionwebservice-1.0.0/examples/googlesearch/delegated/google_search_service.rb
%{gemdir}/gems/actionwebservice-1.0.0/examples/googlesearch/delegated/search_controller.rb
%{gemdir}/gems/actionwebservice-1.0.0/examples/googlesearch/direct/google_search_api.rb
%{gemdir}/gems/actionwebservice-1.0.0/examples/googlesearch/direct/search_controller.rb
%{gemdir}/gems/actionwebservice-1.0.0/examples/metaWeblog/README
%{gemdir}/gems/actionwebservice-1.0.0/examples/metaWeblog/apis/blogger_api.rb
%{gemdir}/gems/actionwebservice-1.0.0/examples/metaWeblog/apis/blogger_service.rb
%{gemdir}/gems/actionwebservice-1.0.0/examples/metaWeblog/apis/meta_weblog_api.rb
%{gemdir}/gems/actionwebservice-1.0.0/examples/metaWeblog/apis/meta_weblog_service.rb
%{gemdir}/gems/actionwebservice-1.0.0/examples/metaWeblog/controllers/xmlrpc_controller.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/api.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/base.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/casting.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/client.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/container.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/dispatcher.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/invocation.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/protocol.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/scaffolding.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/struct.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/test_invoke.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/version.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/client/base.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/client/soap_client.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/client/xmlrpc_client.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/container/action_controller_container.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/container/delegated_container.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/container/direct_container.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/dispatcher/abstract.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/dispatcher/action_controller_dispatcher.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/protocol/abstract.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/protocol/discovery.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/protocol/soap_protocol.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/protocol/xmlrpc_protocol.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/protocol/soap_protocol/marshaler.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/support/class_inheritable_options.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/support/signature_types.rb
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/templates/scaffolds/layout.rhtml
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/templates/scaffolds/methods.rhtml
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/templates/scaffolds/parameters.rhtml
%{gemdir}/gems/actionwebservice-1.0.0/lib/action_web_service/templates/scaffolds/result.rhtml
%{gemdir}/gems/actionwebservice-1.0.0/test/abstract_client.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/abstract_dispatcher.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/abstract_unit.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/api_test.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/base_test.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/casting_test.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/client_soap_test.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/client_xmlrpc_test.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/container_test.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/dispatcher_action_controller_soap_test.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/dispatcher_action_controller_xmlrpc_test.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/gencov
%{gemdir}/gems/actionwebservice-1.0.0/test/invocation_test.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/run
%{gemdir}/gems/actionwebservice-1.0.0/test/scaffolded_controller_test.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/struct_test.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/test_invoke_test.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/apis/auto_load_api.rb
%{gemdir}/gems/actionwebservice-1.0.0/test/apis/broken_auto_load_api.rb
%doc %{gemdir}/doc/actionwebservice-1.0.0
%{gemdir}/cache/actionwebservice-1.0.0.gem
%{gemdir}/specifications/actionwebservice-1.0.0.gemspec

%changelog
* Sat Jan 28 2006 David Lutterkort <dlutter@redhat.com> - 1.0.0-1
- Initial build
