#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# set up the load path
thisdir = File.dirname(__FILE__)
$:.unshift(thisdir)
$:.unshift(File.join(thisdir, ".."))
$:.unshift(File.join(thisdir, "..", ".."))

require 'test_helper'
require 'gateway'


class GatewayTest < Test::Unit::TestCase
  SIPXCONFIG_DATABASE = 'SIPXCONFIG'
  GATEWAY_TABLE_NAME = 'gateway'

  def setup
    # Gateways are stored in the SIPXCONFIG database. Connect to that database.
    # Reuse an existing open connection if one is open. The hitch is that we
    # can't simply use the SIPXCDR connection inherited from ActiveRecord::Base,
    # we need a SIPXCONFIG connection. Reuse the open connection only if it has
    # a gateway table, which indicates a connection to a SIPXCONFIG database.
    if !Gateway.connected? or
       !Gateway.connection.tables.find {|table| table == GATEWAY_TABLE_NAME}
      Gateway.establish_connection(
        :adapter  => "postgresql",
        :host     => "localhost",
        :username => "postgres",
        :database => SIPXCONFIG_DATABASE)
    end
  end
  
  def test_save_load_gateways
    test_gateway_name = 'gateway_for_call_resolver_unit_test'
    
    # Scrub the gateway if it is already there
    gateway = Gateway.find(:first, :conditions => "name = '#{test_gateway_name}'")
    if gateway
      gateway.destroy
    end
    
    # Make a new gateway
    
    # Pick a unique ID for it.  We have to do this ourselves because sipXconfig
    # tables don't follow the ActiveRecord convention that the primary key is
    # named "id".  The primary key in this case is "gateway_id".
    ids = Gateway.find_by_sql("select max(gateway.gateway_id) from gateway")
    new_id = ids[0].max.to_i + 1
    
    # Create the gateway.  Assign the ID in a separate step because that's the
    # only way I could make this work.  Note that even though the ID name is
    # "gateway_id", ActiveRecord always sets the primary key through an id=
    # method regardless of the actual name of the key.
    test_bean_id = 'gwGeneric'
    test_gateway_address = '10.1.1.1'
    test_serial_number = '0123456789AB'
    gateway = Gateway.new(:name => test_gateway_name,
                          :address => test_gateway_address,
                          :serial_number => test_serial_number,
                          :bean_id => test_bean_id)
    gateway.id = new_id
    gateway.save
    
    # Load it
    gateway2 = Gateway.find(:first, :conditions => "name = '#{test_gateway_name}'")
    assert_equal(gateway.name, gateway2.name)
    assert_equal(gateway.address, gateway2.address)
    assert_equal(gateway.serial_number, gateway2.serial_number)
    assert_equal(gateway.bean_id, gateway2.bean_id)
  end
end
