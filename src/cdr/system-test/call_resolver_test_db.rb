#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'test/unit'
require 'rubygems'

$:.unshift(File.join(File.dirname(__FILE__), '..', 'lib'))

require 'call_resolver'

class CallResolverTest < Test::Unit::TestCase

  
  def test_resolve()
    resolver = CallResolver.new()
    
    #resolver.resolve(nil, Time.parse("10/26"))
    resolver.resolve(nil, nil)
  end  
end
