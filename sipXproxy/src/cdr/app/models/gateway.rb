#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# System requires
require 'ipaddr'

# Application requires.  Assume that the load path includes this file's dir.
require 'exceptions'
require 'socket_utils'


# Gateway models a gateway defined in the sipXconfig database. Gateways in the
# database can be given addresses that are either domain names or IP addresses.
# Resolve domain names to IP addresses so that the address will be in a
# canonical form for matching. Assume that the gateway address will not change
# while this process is running, so we can cache the DNS lookup.
class Gateway < ActiveRecord::Base
  # The ActiveRecord convention is that the ID column is always named "id".
  # The sipXconfig convention is that the ID is named tablename_id.  So we
  # need to tell ActiveRecord the primary key name.
  set_primary_key('gateway_id')

  # Tell ActiveRecord that the Gateway class maps to the 'gateway' DB table.
  # ActiveRecord figures that out automatically for tables that follow the
  # ActiveRecord naming convention of having a plural noun as the table name,
  # but the sipXconfig convention is to have singular nouns as table names.
  set_table_name('gateway')

public
  
  def ip_address
    if !@ip_address
      # If the gateway address is a domain name, then resolve it to an IP addr.
      addr = self.address
      if addr
        if SocketUtils.valid_ipaddr?(addr)
          # The gateway address is an IP address.
          @ip_address = addr
        else
          # The gateway address is not an IP address, so it must be a domain name.
          # Try to resolve it.
          @ip_address = ip_address_from_domain_name(addr)
        end
      end
    end
    
    @ip_address
  end
  
private
  
  def ip_address_from_domain_name(domain_name)
    ip_address = nil
    begin
      ip_address = IPSocket.getaddress(domain_name)
    rescue SocketError
      raise(GatewayNameResolutionException.new(self),
            "Unable to resolve the gateway domain name \"#{domain_name}\" to an IP address: #{$!}",
            caller)
    end
    ip_address
  end
  
end





