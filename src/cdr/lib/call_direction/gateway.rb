#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# System requires
require 'dbi'
require 'ipaddr'

# Application requires.  Assume that the load path includes this file's dir.
require 'utils/exceptions'
require 'utils/sipx_ipsocket'
require 'utils/socket_utils'
require 'utils/utils'

SIPXCONFIG_DATABASE = 'SIPXCONFIG'


# The Gateway class models a gateway defined in the sipXconfig database. Gateways in the
# database can be given addresses that are either domain names or IP addresses.
# Resolve domain names to IP addresses so that the address will be in a
# canonical form for matching. Assume that the gateway address will not change
# while this process is running, so we can cache the DNS lookup.
class Gateway
  public
  
  def initialize(address)
    @address = address
  end
  
  def ip_addresses
    if !@ip_addresses
      # If the gateway address is a domain name, then resolve it to an IP addr.
      @ip_addresses = []
      addr = @address
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
  
  class << self
    def find_all
      begin
        gateways = []
        # connect to the MySQL server
        dbh = DBI.connect("dbi:Pg:#{SIPXCONFIG_DATABASE}:localhost", "postgres")
        # get server version string and display it
        sth = dbh.execute("SELECT address FROM gateway")
        sth.each do |row|
          gateways << Gateway.new(row[0])
        end
        sth.finish
        return gateways
      rescue DBI::DatabaseError => e
        puts "An error occurred"
        puts "Error code: #{e.err}"
        puts "Error message: #{e.errstr}"
      ensure
        # disconnect from server
        dbh.disconnect if dbh
      end    
    end
    
    
  end
  
end
