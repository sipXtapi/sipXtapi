# Generated from actionpack-1.11.2.gem by gem2spec -*- rpm-spec -*-
%define rbname actionpack
%define version 1.11.2
%define release 1

Summary: Web-flow and rendering framework putting the VC in MVC.
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
Requires: ruby-gems-activesupport = 1.2.5
BuildRequires: ruby 
BuildRequires: ruby-gems >= 0.8.11
BuildArch: noarch
Provides: ruby(Actionpack) = %{version}

%define gemdir /usr/lib/ruby/gems/1.8
%define gembuilddir %{buildroot}%{gemdir}

%description
Eases web-request routing, handling, and response as a half-way front, half-way page controller. Implemented with specific emphasis on enabling easy unit/integration testing that doesn't require a browser.
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
%{gemdir}/gems/actionpack-1.11.2/rakefile
%{gemdir}/gems/actionpack-1.11.2/install.rb
%{gemdir}/gems/actionpack-1.11.2/README
%{gemdir}/gems/actionpack-1.11.2/RUNNING_UNIT_TESTS
%{gemdir}/gems/actionpack-1.11.2/CHANGELOG
%{gemdir}/gems/actionpack-1.11.2/MIT-LICENSE
%{gemdir}/gems/actionpack-1.11.2/examples/.htaccess
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_pack.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/assertions.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/base.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/benchmarking.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/caching.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/cgi_process.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/code_generation.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/components.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/cookies.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/dependencies.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/deprecated_assertions.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/deprecated_redirects.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/filters.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/flash.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/helpers.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/layout.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/pagination.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/request.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/rescue.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/response.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/routing.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/scaffolding.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/session_management.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/streaming.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/test_process.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/upload_progress.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/url_rewriter.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/verification.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/cgi_ext/cgi_ext.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/cgi_ext/cgi_methods.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/cgi_ext/cookie_performance_fix.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/cgi_ext/multipart_progress.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/cgi_ext/raw_post_data_fix.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/macros/auto_complete.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/macros/in_place_editing.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/session/active_record_store.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/session/drb_server.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/session/drb_store.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/session/mem_cache_store.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/templates/rescues/_request_and_response.rhtml
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/templates/rescues/_trace.rhtml
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/templates/rescues/diagnostics.rhtml
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/templates/rescues/layout.rhtml
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/templates/rescues/missing_template.rhtml
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/templates/rescues/routing_error.rhtml
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/templates/rescues/template_error.rhtml
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/templates/rescues/unknown_action.rhtml
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/templates/scaffolds/edit.rhtml
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/templates/scaffolds/layout.rhtml
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/templates/scaffolds/list.rhtml
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/templates/scaffolds/new.rhtml
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/templates/scaffolds/show.rhtml
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/vendor/xml_simple.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/vendor/html-scanner/html/document.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/vendor/html-scanner/html/node.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/vendor/html-scanner/html/node.rb.rej
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/vendor/html-scanner/html/tokenizer.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_controller/vendor/html-scanner/html/version.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_pack/version.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/base.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/compiled_templates.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/partials.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/template_error.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/active_record_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/asset_tag_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/benchmark_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/cache_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/capture_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/date_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/debug_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/form_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/form_options_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/form_tag_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/java_script_macros_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/javascript_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/number_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/pagination_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/tag_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/text_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/upload_progress_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/url_helper.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/javascripts/controls.js
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/javascripts/dragdrop.js
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/javascripts/effects.js
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/helpers/javascripts/prototype.js
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/vendor/builder.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/vendor/builder/blankslate.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/vendor/builder/xmlbase.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/vendor/builder/xmlevents.rb
%{gemdir}/gems/actionpack-1.11.2/lib/action_view/vendor/builder/xmlmarkup.rb
%{gemdir}/gems/actionpack-1.11.2/test/abstract_unit.rb
%{gemdir}/gems/actionpack-1.11.2/test/testing_sandbox.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/action_pack_assertions_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/active_record_assertions_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/active_record_store_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/addresses_render_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/base_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/benchmark_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/caching_filestore.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/capture_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/cgi_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/components_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/cookie_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/custom_handler_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/fake_controllers.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/filters_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/flash_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/fragment_store_setting_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/multipart_progress_testx.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/new_render_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/raw_post_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/redirect_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/render_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/request_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/routing_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/send_file_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/session_management_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/test_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/upload_progress_testx.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/url_rewriter_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/controller/verification_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/addresses/list.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/fun/games/hello_world.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/helpers/abc_helper.rb
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/helpers/fun/games_helper.rb
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/helpers/fun/pdf_helper.rb
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/layouts/builder.rxml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/layouts/standard.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/layouts/talk_from_action.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/layouts/yield.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/multipart/binary_file
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/multipart/large_text_file
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/multipart/mixed_files
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/multipart/single_parameter
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/multipart/text_file
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/scope/test/modgreet.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/_customer.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/_customer_greeting.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/_hash_object.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/_partial_only.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/_person.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/action_talk_to_layout.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/capturing.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/content_for.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/greeting.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/hello.rxml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/hello_world.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/hello_xml_world.rxml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/list.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/potential_conflicts.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/render_file_with_ivar.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/render_file_with_locals.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/render_to_string_test.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/fixtures/test/update_element_with_capture.rhtml
%{gemdir}/gems/actionpack-1.11.2/test/template/active_record_helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/asset_tag_helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/benchmark_helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/compiled_templates_tests.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/date_helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/form_helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/form_options_helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/form_tag_helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/java_script_macros_helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/javascript_helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/number_helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/tag_helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/text_helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/upload_progress_helper_testx.rb
%{gemdir}/gems/actionpack-1.11.2/test/template/url_helper_test.rb
%{gemdir}/gems/actionpack-1.11.2/examples/address_book_controller.cgi
%{gemdir}/gems/actionpack-1.11.2/examples/address_book_controller.fcgi
%{gemdir}/gems/actionpack-1.11.2/examples/address_book_controller.rb
%{gemdir}/gems/actionpack-1.11.2/examples/address_book_controller.rbx
%{gemdir}/gems/actionpack-1.11.2/examples/benchmark.rb
%{gemdir}/gems/actionpack-1.11.2/examples/benchmark_with_ar.fcgi
%{gemdir}/gems/actionpack-1.11.2/examples/blog_controller.cgi
%{gemdir}/gems/actionpack-1.11.2/examples/debate_controller.cgi
%{gemdir}/gems/actionpack-1.11.2/examples/address_book/index.rhtml
%{gemdir}/gems/actionpack-1.11.2/examples/address_book/layout.rhtml
%{gemdir}/gems/actionpack-1.11.2/examples/debate/index.rhtml
%{gemdir}/gems/actionpack-1.11.2/examples/debate/new_topic.rhtml
%{gemdir}/gems/actionpack-1.11.2/examples/debate/topic.rhtml
%doc %{gemdir}/doc/actionpack-1.11.2
%{gemdir}/cache/actionpack-1.11.2.gem
%{gemdir}/specifications/actionpack-1.11.2.gemspec

%changelog
