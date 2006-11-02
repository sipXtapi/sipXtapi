#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'monitor'
require 'thread'

require 'cdr'
require 'call_state_event'

# Maintains currently processed CDRs.
class State
  include MonitorMixin

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
  
  def initialize(cse_queue, cdr_queue, cdr_class = Cdr)
    @cse_queue = cse_queue
    @cdr_queue = cdr_queue
    @cdr_class = cdr_class
    @failed_calls = {}
    @retired_calls = {}
    @generation = 0
    @cdrs = {}
    super()
  end

  def run
    while cse = @cse_queue.deq
      synchronize do 
        accept(cse)
      end        
    end
  end
    
  # Analyze CSE, if you have cDR completed than return it, otherwise return nil
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
    synchronize do
      @retired_calls.delete_if do | key, value | 
        @generation - value >= age
      end      
    end      
  end
  
  # Call to notify listeners about calls that failed. 
  # We cannot really determine easily if the call had failed, since we do not not if we already process all the call legs.
  # That's why we store potential failures for a while and only notify observers when we done.
  def flush_failed(age)
    synchronize do
      @failed_calls.delete_if do |key, value|
        notify(value.cdr) if @generation - value.generation >= age
      end
    end      
  end
  
  private
  # Add readry CDR to CDR queue, save it in retired_calls collection so that we can ignore new events for this CDR
  def notify(cdr)
    @retired_calls[cdr.call_id] = @generation
    @cdr_queue.enq(cdr)
  end
end
