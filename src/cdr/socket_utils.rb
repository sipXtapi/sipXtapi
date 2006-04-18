#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

class SocketUtils
  
  # SocketUtils has only class methods, so don't allow instantiation.
  private_class_method :new

public
  
  def SocketUtils.ip_address_from_domain_name(domain_name)
    ip_address = nil
    begin
      ip_address = IPSocket.getaddress(domain_name)
    rescue SocketError
      raise(NameResolutionException.new(domain_name),
            "Unable to resolve the domain name \"#{domain_name}\" to an IP address: #{$!}",
            caller)
    end
    ip_address
  end
  
  def SocketUtils.strip_v4_port(addr)
    # Look for string with IPv4 format and a colon with a trailing number
    if /\A(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):\d+\Z/ =~ addr
      return $1
    else
      return addr
    end
  end

end
