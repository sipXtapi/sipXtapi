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
require 'sipx_ipsocket'
require 'socket_utils'
require 'utils'


# The Gateway class models a gateway defined in the sipXconfig database. Gateways in the
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
  
  def ip_addresses
    if !@ip_addresses
      # If the gateway address is a domain name, then resolve it to an IP addr.
      @ip_addresses = []
      addr = self.address
      if addr
        # Strip a possible port number off the IPv4 address
        # LATER - handle IPv6
        addr = SocketUtils.strip_v4_port(addr)
        if SipxIPSocket.valid_ipaddr?(addr)
          # The gateway address is an IP address.
          @ip_addresses << addr
        else
          # Strip a possible port number from domain name
          if /\A(.+):\d+\Z/ =~ addr
            addr = $1
          end          
          # The gateway address is not an IP address, so it must be a domain name.
          # Try to resolve it.
          @ip_addresses.concat(SocketUtils.getaddresses(addr))
        end
      end
    end
    
    @ip_addresses
  end
  
end





