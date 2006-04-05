#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

class CallDirection < ActiveRecord::Base
  # Map the call_directions table to the cdrs table.
  belongs_to :cdr,
             :foreign_key => "id"

  # Call direction values.  The single-char codes are stored in the DB column.
  INCOMING = 'I'        # calls that come in from a PSTN gateway
  OUTGOING = 'O'        # calls that go out to a PSTN gateway
  INTRANETWORK = 'A'    # calls that are pure SIP and don't go through a gateway
end

# Map the cdrs table to the call_directions table.
# Do this separately from the Cdr class declaration because call direction is a
# customer-specific add-on.  It's cool that Ruby lets us dynamically change an
# existing class, so we can stick this code in this plugin file without polluting
# the generic Call Resolver.
class Cdr
  has_one :call_direction,
          :foreign_key => "id"
end
