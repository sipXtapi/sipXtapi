#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'test/unit'

$:.unshift File.join(File.dirname(__FILE__), "..", "lib")
require 'state'


class StateTest < Test::Unit::TestCase
  
  class DummyCdr
    attr_reader :counter
    
    def initialize(call_id)
      @counter = 0
    end
    
    def accept(cse)
      @counter += 1
      return @counter > 1        
    end 
    
  end
  
  
  class DummyObserver
    attr_reader :counter
    
    def initialize()
      @counter = 0
    end
    
    def update(cdr)
      @counter += 1
    end
  end
  
  class DummyCse
    attr_reader :call_id
    
    def initialize(call_id)
      @call_id = call_id
    end 
  end
  
  def test_accept
    observer = DummyObserver.new
    cse1 = DummyCse.new('id1')
    cse2 = DummyCse.new('id2')
    
    state = State.new([], [ observer ], DummyCdr )
    
    state.accept(cse1)    
    assert_equal(0, observer.counter)
    
    state.accept(cse2)
    assert_equal(0, observer.counter)
    
    state.accept(cse1)    
    assert_equal(1, observer.counter)
    
    state.accept(cse2)
    assert_equal(2, observer.counter)    
  end
  
  def test_accept_many_observers
    observer1 = DummyObserver.new
    observer2 = DummyObserver.new
    cse1 = DummyCse.new('id1')
    
    state = State.new([], [ observer1, observer2 ], DummyCdr )
    
    state.accept(cse1)
    assert_equal(0, observer1.counter)
    assert_equal(0, observer2.counter)
    
    state.accept(cse1)
    assert_equal(1, observer1.counter)
    assert_equal(1, observer2.counter)
  end
  
  
end
