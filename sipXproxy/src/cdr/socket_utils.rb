# This file is intentionally lacking the standard SIPfoundry copyright header,
# because the code below is really part of Ruby. This is a temporary hack.
# Copied these methods from the Ruby file ipaddr.rb, which has the code for
# these methods, but inexplicably hides them so they can't be used. Add "ipaddr"
# to the method names for clarity.
# :TODO: post to Ruby forums and find out how to fix this.

class SocketUtils
  
  # SocketUtils has only class methods, so don't allow instantiation.
  private_class_method :new

public
 
  def SocketUtils.valid_v4_ipaddr?(addr)
    if /\A(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})\Z/ =~ addr
      return $~.captures.all? {|i| i.to_i < 256}
    end
    return false
  end

  def SocketUtils.valid_v6_ipaddr?(addr)
    # IPv6 (normal)
    return true if /\A[\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*\Z/ =~ addr
    return true if /\A[\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*::([\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*)?\Z/ =~ addr
    return true if /\A::([\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*)?\Z/ =~ addr
    # IPv6 (IPv4 compat)
    return true if /\A[\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*:/ =~ addr && valid_v4?($')
    return true if /\A[\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*::([\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*:)?/ =~ addr && valid_v4?($')
    return true if /\A::([\dA-Fa-f]{1,4}(:[\dA-Fa-f]{1,4})*:)?/ =~ addr && valid_v4?($')
     false
  end

  def SocketUtils.valid_ipaddr?(addr)
    valid_v4_ipaddr?(addr) || valid_v6_ipaddr?(addr)
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
