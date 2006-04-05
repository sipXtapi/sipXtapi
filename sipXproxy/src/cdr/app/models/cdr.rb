#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

class Cdr < ActiveRecord::Base
  
public  
  # Describe the mapping from the cdrs table to the parties table.
  # Because the cdrs table references the parties table twice, once for the caller
  # and once for the callee, we have to specify details that ActiveRecord would
  # typically figure out automatically.
  belongs_to :caller,
             :class_name => "Party",
             :foreign_key => "caller_id"
  belongs_to :callee,
             :class_name => "Party",
             :foreign_key => "callee_id"
  
  # Constants representing termination codes
  CALL_REQUESTED_TERM   = 'R'
  CALL_IN_PROGRESS_TERM = 'I'
  CALL_COMPLETED_TERM   = 'C'
  CALL_FAILED_TERM      = 'F'
  
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
