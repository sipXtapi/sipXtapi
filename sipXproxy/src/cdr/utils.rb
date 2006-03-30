#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require File.join(File.dirname(__FILE__), 'exceptions')

class Utils
  # Utils has only class methods, so don't allow instantiation.
  private_class_method :new

public
  # Return just the AOR part of a SIP From or To header, stripping the tag if
  # one is present.
  # If there is no tag then raise BadSipHeaderException if is_tag_required is true.
  def Utils.get_aor_from_header(header, is_tag_required = true)
    aor = nil
    
    # find the semicolon preceding the tag
    semi = header.index(';')
    
    # extract the AOR
    if semi
      aor = header[0, semi]
    else
      if is_tag_required
        raise(BadSipHeaderException. new(header),
              'Tag missing from SIP header',
              caller)
      end
      aor = header
    end
    
    aor
  end

end
