#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# system requires
require 'rubygems'            # Ruby packaging and installation framework
require_gem 'activerecord'    # object-relational mapping layer for Rails
require 'logger'              # writes log messages to a file or stream

# application requires
require File.dirname(__FILE__) + '/app/models/call_state_event'
require File.dirname(__FILE__) + '/app/models/cdr'
require File.dirname(__FILE__) + '/app/models/party'


# The CallResolver analyzes call state events (CSEs) and computes call detail 
# records (CDRs).  It loads CSEs from a database and writes CDRs back into the
# same database.
class CallResolver

public

  def initialize
    # Set up logging.
    # :TODO: set up standard log entry format, e.g., include a timestamp
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
  # :TODO: Open a database connection if one is not already open.
  def resolve(start_time, end_time, redo_flag = false)
    # If end_time is not provided, then set it to 1 day after the start time.
    end_time ||= start_time.next

    log.info("resolve: start = #{start_time.to_s}, end = #{end_time.to_s},
             redo = #{redo_flag}")

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
  # :NOW: catch non-fatal exceptions thrown by Call Resolver code.  Discard the
  # CDR when such exceptions happen and log an error.
  def resolve_call(call_id)
    log.debug("Resolving a call: call ID = #{call_id}")
    
    # Load all events with this call_id, in ascending chronological order.
    # Don't constrain the query to a time window.  That allows us to handle
    # calls that span time windows.
    events = load_events(call_id)
    
    # Find the first (earliest) call request event.
    call_req = find_first_call_request(events)
    if call_req
      # Read info from it and start constructing the CDR.
      cdr_data = read_call_request(call_req)
      
      # The forking proxy might ring multiple phones.  The dialog with each
      # phone is a separate call leg.
      # Pick the call leg with the best outcome and longest duration to be the
      # basis for the CDR.
      to_tag = best_call_leg(events)
      
      if to_tag                         # if there are any complete call legs
        # Fill the CDR from the call leg events.  The returned status is true
        # if that succeeded, false otherwise.
        status = create_cdr(cdr_data, events, to_tag)
      
        if status
          if log.debug?
            cdr = cdr_data.cdr
            log.debug("Resolved a call from #{cdr_data.caller.aor} to " +
                      "#{cdr_data.callee.aor} at #{cdr.start_time}, status = " +
                      "#{cdr.termination_text}")
          end
          
          # Save the CDR and associated data, within a transaction.
          save_cdr(cdr_data)
        end
      end
    end
  end
    
  # Load all events with the given call_id, in ascending chronological order.
  def load_events(call_id)
    events =
      CallStateEvent.find(
        :all,
        :conditions => ["call_id = :call_id", {:call_id => call_id}],
        :order => "event_time")
    log.debug("load_events: loaded #{events.length} events with call_id = #{call_id}")
    return events
  end
  
  # Find and return the first (earliest) call request event.
  # Return nil if there is no such event.
  def find_first_call_request(events)
    event = events.find {|event| event.call_request?}

    if log.debug?
      message = event ? "found #{event}" : "no call request found"
      log.debug('find_first_call_request: ' + message);
    end
    
    return event
  end

  # Read info from the call request event and start constructing the CDR.
  # Return the new CDR.
  def read_call_request(call_req)
    caller = Party.new(:aor => call_req.caller_aor,
                       :contact => call_req.contact)
    callee = Party.new(:aor => call_req.callee_aor)
    cdr = Cdr.new(:call_id => call_req.call_id,
                  :from_tag => call_req.from_tag,
                  :start_time => call_req.event_time,
                  :termination => Cdr::CALL_REQUESTED_TERM)
    CdrData.new(cdr, caller, callee)
  end

  # Pick the call leg with the best outcome and longest duration to be the
  # basis for the CDR.  Return the to_tag for that call leg.  A call leg is a
  # set of events with a common to_tag.
  # Return nil if there is no such call leg.
  def best_call_leg(events)     # array of events with a given call ID
    to_tag = nil                # result: the to_tag for the best call leg
    
    # If there are no call_end events, then the call failed
    call_failed = !events.any? {|event| event.call_end?}
    
    # Find the call leg with the best outcome and longest duration.
    # If the call succeeded, then look for the call end event with the biggest
    # timestamp.  Otherwise look for the call failure event with the biggest
    # timestamp.  Events have already been sorted for us in timestamp order.
    final_event_type = call_failed ?
                       CallStateEvent::CALL_FAILURE_TYPE :
                       CallStateEvent::CALL_END_TYPE
    events.reverse_each do |event|
      if event.event_type == final_event_type
        to_tag = event.to_tag
        break
      end
    end
    
    if !to_tag
      # If there is no final event, then try to at least find a call_setup event
      events.reverse_each do |event|
        if event.call_setup?
          to_tag = event.to_tag
          break
        end
      end
    end

    to_tag
  end
  
  # Fill in the CDR from a call leg consisting of the events with the given
  # to_tag.  Return true if successful, false otherwise.
  def create_cdr(cdr_data, events, to_tag)
    status = false                # return value: did we fill in the CDR?
    
    # get the events for the call leg
    call_leg = events.find_all {|event| event.to_tag == to_tag}
    
    # find the call_setup event
    call_setup = call_leg.find {|event| event.call_setup?}
    if call_setup
      cdr = cdr_data.cdr
      
      # The call was set up, so mark it provisionally as in progress.
      cdr.termination = Cdr::CALL_IN_PROGRESS_TERM
 
      # We have enough data now to build the CDR.
      status = true
      
      # get data from the call_setup event
      cdr_data.callee.contact = call_setup.contact
      cdr.to_tag = call_setup.to_tag
      cdr.connect_time = call_setup.event_time
      
      # get data from the call_end or call_failure event
      call_end = call_leg.find {|event| event.call_end?}
      if call_end
        cdr.termination = Cdr::CALL_COMPLETED_TERM    # successful completion
        cdr.end_time = call_end.event_time
      else
        # Couldn't find a call_end, try for call_failure
        call_failure = call_leg.find {|event| event.call_failure?}
        if call_failure
          # found a call_failure event, use it
          # :TODO: consider optimizing space usage by not setting the
          # failure_reason if it is the default value for the failure_status
          # code. For example, the 486 error has the default reason "Busy Here".
          cdr.termination = Cdr::CALL_FAILED_TERM
          cdr.end_time = call_failure.event_time
          cdr.failure_status = call_failure.failure_status
          cdr.failure_reason = call_failure.failure_reason
        end
      end
    end
    
    status
  end

  # Save the CDR and associated data, within a transaction.
  # Be sure not to duplicate existing data.  For example, the caller
  # may already be present in the parties table from a previous call,
  # or a complete CDR may have been created for this call in a
  # previous run.
  # Raise a CallResolverException if the save fails for some reason.
  # :TODO: support the redo flag for recomputing CDRs
  def save_cdr(cdr_data)
    # define variables for cdr_data components
    cdr = cdr_data.cdr
    caller = cdr_data.caller
    callee = cdr_data.callee
    
    Cdr.transaction do
      # Continue only if a complete CDR doesn't already exists.
      # :TODO: Check for race condition allowing CDR to sneak into DB,
      # triggering exception on save -- see save_party_if_new.
      # :NOW: The case of an incomplete CDR is not being handled right
      # since we will try to save a duplicate CDR, fix that.
      db_cdr = find_cdr_by_dialog(cdr.call_id, cdr.from_tag, cdr.to_tag)
      if (!db_cdr or !db_cdr.complete?)
      
        # save the caller and callee if they don't already exist
        caller = save_party_if_new(caller)
        callee = save_party_if_new(callee)
        cdr.caller_id = caller.id
        cdr.callee_id = callee.id
        
        # save the CDR
        if !cdr.save
          raise(CallResolverException, 'save_cdr: save failed', caller)
        end
      end
    end
  end
  
  # Given the parameters identifying a SIP dialog -- call_id, from_tag, and
  # to_tag -- return the CDR, or nil if a CDR could not be found.
  def find_cdr_by_dialog(call_id, from_tag, to_tag)
    Cdr.find(
      :first,
      :conditions =>
        ["call_id = :call_id and from_tag = :from_tag and to_tag = :to_tag",
         {:call_id => call_id, :from_tag => from_tag, :to_tag => to_tag}])
  end
  
  # Given an in-memory Party, find that Party in the database and return it.
  # If the Party is not in the database, then return nil.
  def find_party(party)
    Party.find_by_aor_and_contact(party.aor, party.contact)
  end
  
  # Save the input Party if it is not already in the database.
  # Return the Party, either the saved input Party or the Party with equal
  # values that was already in the database.
  def save_party_if_new(party)
    party_in_db = find_party(party)
    if !party_in_db
      begin
        if party.save
          party_in_db = party
        else
          raise(CallResolverException, 'save_party_if_new: save failed', caller)
        end
      rescue ActiveRecord::StatementInvalid
        # Because we are doing optimistic locking, there is a small chance that
        # a Party got saved just after we did the check above, resulting in a
        # DB integrity violation when we try to save a duplicate.  In this case
        # return the Party that is in the database.
        log.debug("save_party_if_new: unusual race condition detected")
        party_in_db = find_party(party)
        
        # The Party had better be in the database now.  If not then rethrow,
        # we are lost in the woods without a flashlight.
        if !party_in_db
          log.error("save_party_if_new: party should be in database but isn't")
          raise
        end
      end
    end
    
    party_in_db
  end
  
end    # class CallResolver


# CdrData holds data for a CDR that is under construction: the CDR and the
# associated caller and callee parties.  Because this data has not yet been
# written to the DB, we can't use the foreign key relationships that usually
# link the CDR to the caller and callee.
class CdrData
  attr_accessor :cdr, :caller, :callee

  def initialize(cdr = Cdr.new, caller = Party.new, callee = Party.new)
    @cdr = cdr
    @caller = caller
    @callee = callee
  end
end
