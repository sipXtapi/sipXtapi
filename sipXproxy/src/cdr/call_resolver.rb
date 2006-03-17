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
      @@log.level = Logger::DEBUG
#    else
#      @@log.level = Logger::INFO
#    end
  end

  # Resolve CSEs to CDRs
  def resolve(start_time, end_time, redo_flag)
    # If end_time is not provided, then set it to 1 day after the start time.
    end_time ||= start_time.next

    @@log.info "resolve: start = #{start_time.to_s}, end = #{end_time.to_s}, redo = #{redo_flag}"

    # Load CSEs from the specified time window, sorted by call_id then by event_time.
    events = load_call_state_events(start_time, end_time)
    
  end

private

  # Load and return CSEs from the specified time window,
  # sorted by call_id then by event_time.
  def load_call_state_events(start_time, end_time)
    events = 
      CallStateEvent.find(:all,
                          :conditions => ["event_time >= :start and event_time <= :end",
                                          {:start => start_time, :end => end_time}],
                          :order => "call_id, event_time")
    @@log.debug("load_call_state_events: loaded #{events.length} events between "\
                + "#{start_time.to_s} and #{end_time.to_s}")
    return events;
  end
  
end    # class CallResolver
