#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# CallResolverException is the base class for all Call Resolver exceptions.
class CallResolverException < StandardError
end

class BadSipHeaderException < CallResolverException
  attr :header
  
  # Construct BadSipHeaderException with the text from the bad header.
  def initialize(header)
    @header = header
  end
  
  def to_s
    super + ": \"#{header}\""
  end
  
end

class ConfigException < CallResolverException
end
