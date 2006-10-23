#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# application requires
require 'utils'


class CallStateEvent

public
  
  # Constants representing event types
  CALL_REQUEST_TYPE =  'R'
  CALL_SETUP_TYPE =    'S'
  CALL_END_TYPE =      'E'
  CALL_TRANSFER_TYPE = 'T'
  CALL_FAILURE_TYPE =  'F'

  def inspect
    to_s
  end

  def to_s
    begin
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
      cseq = self.cseq || missing
      
      # Create and return a human-readable description of the event
      "#<#{event_type_name} from #{from_url} to #{to_url} " +
        "at #{event_time} call_id=#{call_id} cseq=#{cseq}>"
    
    # ActiveRecord queries can return objects that don't have all attributes
    # defined.  In that case fall back to the superclass method.
    rescue NoMethodError
      super.to_s
    end
  end

  # Return the AOR part of the from_url, or nil if there is no from_url.
  # Raise BadSipHeaderException if the tag is missing from the from_url.
  def caller_aor
    from_url = self.from_url
    if from_url
      Utils.get_aor_from_header(from_url)
    else
      nil
    end
  end

  # Return the AOR part of the to_url, or nil if there is no to_url.
  # Raise BadSipHeaderException if the tag is missing from the to_url when
  # it should be there.  All events except call request should have the to tag.
  def callee_aor
    to_url = self.to_url
    if to_url
      Utils.get_aor_from_header(
        to_url,
        self.event_type != CALL_REQUEST_TYPE)   # whether to tag is required
    else
      nil
    end
  end

  # Convenience methods for checking the event type
  def call_request?
    self.event_type == CALL_REQUEST_TYPE
  end
  def call_setup?
    self.event_type == CALL_SETUP_TYPE
  end
  def call_end?
    self.event_type == CALL_END_TYPE
  end
  def call_transfer?
    self.event_type == CALL_TRANSFER_TYPE
  end
  def call_failure?
    self.event_type == CALL_FAILURE_TYPE
  end

private

  # Map event type codes to human-readable strings
  EVENT_TYPE_NAMES = { CALL_REQUEST_TYPE =>  'call_request',
                       CALL_SETUP_TYPE =>    'call_setup',
                       CALL_END_TYPE =>      'call_end',
                       CALL_TRANSFER_TYPE => 'call_transfer',
                       CALL_FAILURE_TYPE =>  'call_failure' }

end
