#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

class Party < ActiveRecord::Base
  def ==(party)
    self.aor == party.aor && self.contact == party.contact
  end
  
  def eql?(party)
    self == party
  end
  
  def hash
    return self.aor.hash + self.contact.hash
  end
end
