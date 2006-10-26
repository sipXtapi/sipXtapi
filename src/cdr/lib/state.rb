#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'observer'

require 'cdr'
require 'call_state_event'

# Maintains currently processed CDRs.
class State
  
  # Temporary record of the CDR that is a candidate for failure
  # All legs are failed and there are no remaining legs open
  # It really might be a failure or we are still waiting for a successful leg
  class FailedCdr
    attr_reader :cdr, :generation
    
    def initialize(cdr, generation)
      @cdr = cdr
      @generation = generation  
    end
  end
  
  include Observable
  
  def initialize(generators, observers, cdr_class = Cdr)
    @generation = 0
    @cdr_class = cdr_class
    @generators = generators    
    @cdrs = {}
    @failed_calls = {}
    @retired_calls = {}
    @generators.each { |g| g.set_state(self) }
    observers.each { |l| add_observer(l) }
  end
  
  def accept(cse)
    @generation += 1
    call_id = cse.call_id
    
    # ignore if we already notified world about this cdr
    return if @retired_calls[call_id]
    
    # check if this is a message for call that we suspect might be failed
    failed_cdr = @failed_calls.delete(call_id)
    
    cdr = if failed_cdr
      @cdrs[call_id] = failed_cdr.cdr
    else    
      @cdrs[call_id] ||= @cdr_class.new(call_id)
    end      
    
    if cdr.accept(cse)
      @cdrs.delete(call_id)
      if cdr.terminated?
        notify(cdr)
      else
        # delay notification for failed CDRs - they will get another chance
        @failed_calls[call_id] = FailedCdr.new(cdr, @generation)
      end
    end      
  end
  
  # Strictly speaking this function does not have to be called.
  # Since it is possible that we receive notifications after we already 
  # notified about the CDR we need to maintain a list of CD for which we are going to ignore all notifications.
  # If it's never flushed we are still OK but since every CDR has to be checked against this list the performance will suffer.
  def flush_retired(age)
    @retired_calls.delete_if do | key, value | 
      @generation - value >= age
    end      
  end
  
  # Call to notify listeners about calls that failed. 
  # We cannot really determine easily if the call had failed, since we do not not if we already process all the call legs.
  # That's why we store potential failures for a while and only notify observers when we done.
  def flush_failed(age)
    @failed_calls.delete_if do |key, value|
      notify(value.cdr) if @generation - value.generation >= age
    end
  end
  
  private
  # Version of notifify_observers which always sets changed flag and always returns true
  def notify(cdr)
    @retired_calls[cdr.call_id] = @generation
    changed
    notify_observers(cdr)
    true
  end
end
