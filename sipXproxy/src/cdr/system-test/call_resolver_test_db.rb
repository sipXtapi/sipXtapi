#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'test/unit'


$:.unshift(File.join(File.dirname(__FILE__), '..', 'lib'))

require 'call_resolver'
require 'utils/call_resolver_configure'

class CallResolverTest < Test::Unit::TestCase
  def setup
    @config = CallResolverConfigure.from_file()
  end

  
  def test_resolve()
    resolver = CallResolver.new(@config)
    
    #resolver.resolve(nil, Time.parse("10/26"))
    resolver.resolve(nil, nil)
  end  
  
  def _test_daily
    resolver = CallResolver.new(@config)
    resolver.run_resolver
  end
end
