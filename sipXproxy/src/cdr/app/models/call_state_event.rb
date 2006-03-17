#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

class CallStateEvent < ActiveRecord::Base

public

  def inspect
    to_s
  end

  def to_s
    # Map the event type code to a human-readable string
    event_type_name = nil
    if self.event_type
      event_type_name = EVENT_TYPE_NAMES[self.event_type]
      if !event_type_name
        event_type_name = "unknown CSE type '#{self.event_type}'"
      end
    end
    
    # Show '(nil)' for empty values if data is missing
    missing = '(nil)'
    event_type_name = event_type_name || missing
    from_url = self.from_url || missing
    to_url = self.to_url || missing
    event_time = self.event_time || missing
    call_id = self.call_id || missing
    
    # Create and return a human-readable description of the event
    "#{event_type_name} from #{from_url} to #{to_url} " +
      "at #{event_time} call_id=#{call_id}"
  end

private

  # Map event type codes to human-readable strings
  EVENT_TYPE_NAMES = { 'R' => 'call_request',
                       'S' => 'call_setup',
                       'E' => 'call_end',
                       'T' => 'call_transfer',
                       'F' => 'call_failure' }

end
