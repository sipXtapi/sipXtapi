#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# set up the load path
$:.unshift(File.join(File.dirname(__FILE__), "..", ".."))

require 'test/test_helper'
require 'app/models/gateway'

class GatewayTest < Test::Unit::TestCase
  
  def test_load_gateways    
    # Load it
    gateways = Gateway.find_all
    
    # FIXME add some test to it
    assert(gateways.size > 0)
    puts gateways[0].ip_addresses
  end
end
