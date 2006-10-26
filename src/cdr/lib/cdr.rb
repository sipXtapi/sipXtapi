#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'utils/utils'

class CallLeg
  include Comparable
  
  def initialize(to_tag)
    @to_tag = to_tag
  end
  
  attr_accessor :connect_time, :end_time, :status, :to_tag, :callee_contact
  def has_duration?
    @connect_time && @end_time
  end
  
  def completed?
    @status == Cdr::CALL_COMPLETED_TERM
  end
  
  def duration
    raise ArgumentError, "only defined for completed" unless has_duration?
    @end_time - @connect_time
  end
  
  def <=>(anOther)
    return 1 if completed? && !anOther.completed?
    return -1 if !completed? && anOther.completed?
    return duration <=> anOther.duration
  end  
end


class CallLegs
  attr_reader :best_leg
  
  def initialize
    @legs = {}
    @best_leg = nil
  end
  
  def accept_setup(cse)
    leg = get_leg(cse.to_tag)
    leg.callee_contact = Utils.contact_without_params(cse.contact)
    leg.connect_time = cse.event_time
    leg.status = Cdr::CALL_IN_PROGRESS_TERM unless leg.status
    check_best_leg(leg)
  end
  
  def accept_end(cse)
    leg = get_leg(cse.to_tag)
    leg.end_time = cse.event_time
    leg.status = if cse.call_end? then Cdr::CALL_COMPLETED_TERM else Cdr::CALL_FAILED_TERM end
    check_best_leg(leg)
  end
  
  def check_best_leg(leg)
    return unless leg.has_duration?
    @legs.delete(leg.to_tag)
    if !@best_leg || @best_leg < leg
      @best_leg = leg                
    end    
  end
  
  def done?
    return false unless @best_leg
    return @legs.empty? || @best_leg.completed?
  end
  
  private
  def get_leg(to_tag)
    @legs[to_tag] ||= CallLeg.new(to_tag)
  end
end

#
#  CDR description
#  id                                 primary key
#  call_id text not null              SIP call ID from the INVITE 
#  from_tag text not null             SIP from tag provided by caller in the INVITE 
#  to_tag text not null               SIP to tag provided by callee in the 200 OK 
#  caller_aor text not null           caller's SIP address of record (AOR) 
#  callee_aor text not null           callee's AOR 
#  start_time timestamp               Start time in GMT: initial INVITE received 
#  connect_time timestamp             Connect time in GMT: ACK received for 200 OK 
#  end_time timestamp                 End time in GMT: BYE received  or other ending 
#  termination char(1)                Why the call was terminated 
#  failure_status int2                SIP error code if the call failed  e.g.  4xx 
#  failure_reason text                Text describing the reason for a call failure 
#  call_direction char(1)             Plugin feature  see below 
class Cdr
  # Constants representing termination codes
  CALL_REQUESTED_TERM   = 'R'
  CALL_IN_PROGRESS_TERM = 'I'
  CALL_COMPLETED_TERM   = 'C'
  CALL_FAILED_TERM      = 'F'
  
  public  
  
  def initialize(call_id)
    @call_id = call_id
    @got_original = false
    @legs = CallLegs.new
  end
  
  # Make caller_contact and callee_contact available during call resolution,
  # but don't persist them.
  attr_accessor :caller_contact, :callee_contact, :termination
  
  # Return true if the CDR is complete, false otherwise.
  def complete?
    @termination == CALL_COMPLETED_TERM || @termination == CALL_FAILED_TERM
  end
  
  def terminated?
    @termination == CALL_COMPLETED_TERM
  end
  
  # Return a text description of the termination status for this CDR.
  def termination_text
    TERMINATION_NAMES[@termination] if @termination
  end  
  
  def accept(cse)
    case
    when cse.call_request?
      accept_call_request(cse)
    when cse.call_setup?
      accept_call_setup(cse)
    when cse.call_end?, cse.call_failure?
      accept_call_end(cse)
    end
  end
  
  private
  
  # original (without to_tag) request is always better than not original (with to_tag) request
  def accept_call_request(cse)
    original = !cse.to_tag
    # bailout if we already have original request and this one is not
    return if(@got_original && !orginal) 
    
    # continue if we are original or if we are older 
    if((!@got_orginal && original) || !@start_time || @start_time > cse.event_time)
      
      @from_tag = cse.from_tag
      @caller_aor = cse.caller_aor
      @callee_aor = cse.callee_aor
      @start_time = cse.event_time
      @caller_contact = Utils.contact_without_params(cse.contact)
      
      @termination = CALL_REQUESTED_TERM unless @termination
      
      @got_original ||= !cse.to_tag      
    end
    return nil
  end
  
  def accept_call_setup(cse)
    leg = @legs.accept_setup(cse)
    finish
  end
  
  def accept_call_end(cse)
    leg = @legs.accept_end(cse)
    finish
  end
    
  def finish
    return unless @legs.done?
    leg = legs.best_leg
    @to_tag = leg.to_tag
    @callee_contact = leg.callee_contact
    @connect_time = leg.connect_time
    @end_time = leg.end_time
    return self    
  end
  
  # Map termination codes to human-readable strings
  TERMINATION_NAMES = { 
    CALL_REQUESTED_TERM   => 'requested',
    CALL_IN_PROGRESS_TERM => 'in progress',
    CALL_COMPLETED_TERM   => 'completed',
    CALL_FAILED_TERM      => 'failed'}  
end
