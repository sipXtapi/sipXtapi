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
  include Observable
  
  def initialize(generators, observers, cdr_class = Cdr)
    @cdr_class = cdr_class
    @generators = generators
    @cdrs = {}
    @generators.each { |g| g.set_state(self) }
    observers.each { |l| add_observer(l) }
  end
  
  def accept(cse)
    call_id = cse.call_id
    cdr = @cdrs[call_id] ||= @cdr_class.new(call_id)
    changed(cdr.accept(cse))
    if changed?
      notify_observers(cdr)
      @cdrs.delete(call_id)
    end
  end
end
