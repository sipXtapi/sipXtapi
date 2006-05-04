#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

class Cdr < ActiveRecord::Base
  
  # Constants representing termination codes
  CALL_REQUESTED_TERM   = 'R'
  CALL_IN_PROGRESS_TERM = 'I'
  CALL_COMPLETED_TERM   = 'C'
  CALL_FAILED_TERM      = 'F'
  
public  

  # Make caller_contact and callee_contact available during call resolution,
  # but don't persist them.
  attr_accessor :caller_contact, :callee_contact
  
  # Return true if the CDR is complete, false otherwise.
  def complete?
    is_complete = false
    termination = self.termination
    if termination
      is_complete = (termination == CALL_COMPLETED_TERM) ||
                    (termination == CALL_FAILED_TERM)
    end
    is_complete
  end
  
  # Return a text description of the termination status for this CDR.
  def termination_text
    return nil if !termination
    text = TERMINATION_NAMES[termination]    
  end
  
private

  # Map termination codes to human-readable strings
  TERMINATION_NAMES = { 
    CALL_REQUESTED_TERM   => 'requested',
    CALL_IN_PROGRESS_TERM => 'in progress',
    CALL_COMPLETED_TERM   => 'completed',
    CALL_FAILED_TERM      => 'failed'}
  
end
