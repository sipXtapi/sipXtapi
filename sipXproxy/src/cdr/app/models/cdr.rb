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
  
  # Constants representing termination codes
  CALL_REQUESTED_TERM   = 'R'
  CALL_IN_PROGRESS_TERM = 'I'
  CALL_COMPLETED_TERM   = 'C'
  CALL_FAILURE_TERM     = 'F'
  
end
