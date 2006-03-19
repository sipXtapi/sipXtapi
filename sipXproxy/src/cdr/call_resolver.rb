#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'rubygems'            # Ruby packaging and installation framework
require_gem 'activerecord'    # object-relational mapping layer for Rails
require 'logger'              # writes log messages to a file or stream


# The CallResolver analyzes call state events (CSEs) and computes call detail 
# records (CDRs).  It loads CSEs from a database and writes CDRs back into the
# same database.
class CallResolver

public

  def initialize
    # Set up logging.
    # :TODO: write to $SIPX_PREFIX/var/log/sipxpbx/sipcallresolver.log
    # :TODO: figure out interactions between Ruby log rotation and sipX log rotation
    @@log = Logger.new(STDOUT)

    # Hardwire logging to DEBUG level for now until I figure out why running Ruby
    # in debug mode isn't working.
    # :TODO: fix this.
#    if $DEBUG then
      log.level = Logger::DEBUG
#    else
#      log.level = Logger::INFO
#    end
  end

  # Resolve CSEs to CDRs.
  # :TODO: Support redo_flag.
  def resolve(start_time, end_time, redo_flag)
    # If end_time is not provided, then set it to 1 day after the start time.
    end_time ||= start_time.next

    log.info "resolve: start = #{start_time.to_s}, end = #{end_time.to_s}, redo = #{redo_flag}"

    # Load the call IDs for the specified time window
    call_ids = load_call_ids(start_time, end_time)

    # Resolve each call to yield 0-1 CDRs.  Save the CDRs.
    call_ids.each {|call_id| resolve_call(call_id)}
  end

private

  # Log reader. Provide instance-level accessor to reduce typing.
  def log
    return @@log
  end
  
  # Set up data members.  Use accessors, otherwise derived classes will have no
  # chance to get at this data, should we decide to make any data protected.
  attr_accessor(
    :caller,                       # calling party
    :callee,                       # called party
    :cdr)                          # CDR under construction
  
  # Load the call IDs for the specified time window.
  def load_call_ids(start_time, end_time)
    # This query returns an array of objects of class CallStateEvent, but they
    # only have the call_id attribute defined.  Convert to an array of strings
    # and return that.
    results = CallStateEvent.find_by_sql(
      "select distinct call_id from call_state_events " +
      "where event_time >= '#{start_time}' and event_time <= '#{end_time}'")
    results.collect do |obj|
      obj.call_id
    end
  end
  
  # Resolve the call with the given call_id to yield 0-1 CDRs.  Persist the CDRs.
  # Return true if a CDR was created, false otherwise.
  def resolve_call(call_id)
    made_cdr = false              # assume failure
    init_state                    # clear state variables for CDR processing
    
    # Load all events with this call_id, in ascending chronological order.
    # Don't constrain the query to a time window.  That allows us to handle calls
    # that span time windows.
    events = load_events(call_id)
    
    # Find the first (earliest) call request event.
    call_req = find_first_call_request(events)
    if call_req
      # Read info from it
      read_call_request(call_req)
      
      # Group remaining events into call legs by the to tag.  Construct a hash
      # table where the key is the to tag and the object is an event array
      # containing the events for that call leg.
      # For example, the forking proxy might ring multiple phones.  The dialog
      # with each phone is a separate call leg.
      call_legs = group_call_legs(events)
      
      # Pick the call leg with the best outcome and longest duration to be the
      # basis for the CDR.
      call_leg = best_call_leg(call_legs)
      
      # Create the CDR from the call leg events
      create_cdr
      
      # Save the CDR and associated data, within a transaction.  Be sure not to
      # duplicate existing data.  For example, the caller may already be present
      # in the parties table from a previous call, or a complete CDR may have been
      # created for this call in a previous run.
      made_cdr = save_cdr
    end
    
=begin    
    Create a Dialog instance, dialog. Set dialog.call_id and dialog.from_tag from the event.
Create a Party instance, caller, for the caller:
Set caller.aor to the aor part of the from_url value from the call_request, stripping any tags from the from_url. (Convert aor to a canonical format using Url class?)
Set caller.contact to the contact value of the call_request.
Create a Party instance, callee, for the callee:
Set callee.aor to the aor part of the to_url value from the call_request, stripping any tags from the to_url. 
Create a Cdr instance, cdr:
Set cdr.start_time to the timestamp from the call_request.
Set cdr.termination provisionally to “R” meaning “call requested”.
If no call_request event is found, then bail out.
=end

    return made_cdr
  end
  
  # Clear state variables so that results from one CDR don't pollute the next CDR
  def init_state
    caller = callee = cdr = nil
  end
    
  # Load all events with the given call_id, in ascending chronological order.
  def load_events(call_id)
    events =
      CallStateEvent.find(
        :all,
        :conditions => ["call_id = :call_id", {:call_id => call_id}],
        :order => "event_time")
    log.debug("load_events: loaded #{events.length} events for call_id #{call_id}")
    return events
  end
  
end    # class CallResolver








