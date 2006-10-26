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
    attr_reader :counter, :call_id
    
    def initialize(call_id)
      @counter = 0
      @call_id = call_id
    end
    
    def accept(cse)
      @counter += 1
      return self if @counter > 1        
    end
    
    def terminated?
      true
    end    
  end
  
  
  class DummyObserver
    attr_reader :counter, :last_cdr
    
    def initialize()
      @counter = 0
    end
    
    def update(cdr)
      @last_cdr = cdr
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
  
  def test_retired
    observer = DummyObserver.new
    cse1 = DummyCse.new('id1')
    
    state = State.new([], [ observer ], DummyCdr )
    state.accept(cse1)    
    state.accept(cse1)    
    assert_equal(1, observer.counter)
    assert_equal(2, observer.last_cdr.counter)
    
    state.accept(cse1)
    state.accept(cse1)
    # still only 1 since CDR was retired
    assert_equal(1, observer.counter)
    assert_equal(2, observer.last_cdr.counter)
    
    state.flush_retired(0)
    state.accept(cse1)
    state.accept(cse1)
    # we should be notified again
    assert_equal(2, observer.counter)
    assert_equal(2, observer.last_cdr.counter)
  end
  
  def test_retired_with_age
    observer = DummyObserver.new
    cse1 = DummyCse.new('id1')
    cse2 = DummyCse.new('id2')
    
    state = State.new([], [ observer ], DummyCdr )
    state.accept(cse1)    
    state.accept(cse1)    
    assert_equal(1, observer.counter)
    assert_equal(2, observer.last_cdr.counter)
    # generation == 2
    
    state.accept(cse1)
    state.accept(cse1)
    # still only 1 since CDR was retired
    assert_equal(1, observer.counter)
    assert_equal(2, observer.last_cdr.counter)
    # generation == 4

    state.flush_retired(3) # it's only 2 generations old at his point
    state.accept(cse1)
    state.accept(cse1)
    # nothing was flushed - still one
    assert_equal(1, observer.counter)    
  end
  
  class MockCdr
    @@results = []
    
    attr_reader :call_id
    
    def initialize(call_id)
      @call_id = call_id
    end
    
    def accept(cse) 
      self if @@results.shift
    end
    
    def terminated?
      @@results.shift 
    end
    
    def MockCdr.push_results(*arg)
      @@results.push(*arg)
    end
    
    def MockCdr.results(*arg)
      @@results = arg
    end    
  end
  
  def test_failed
    observer = DummyObserver.new
    cse1 = DummyCse.new('id1')
    
    # results of calls to accept and terminated?
    MockCdr.results( true, false )
    state = State.new([], [ observer ], MockCdr )
    
    state.accept(cse1)
    assert_equal(0, observer.counter)
    
    state.flush_failed(1)
    assert_equal(0, observer.counter)

    state.flush_failed(0)
    assert_equal(1, observer.counter)
    
    MockCdr.push_results( true, true )
    # now we should have a call in retired list so new events will be ignored
    state.accept(cse1)
    assert_equal(1, observer.counter)
  end
  
  
  def test_failed_and_then_succeeded
    observer = DummyObserver.new
    cse1 = DummyCse.new('id1')
    
    # results of calls to accept and terminated?
    MockCdr.results( true, false, true, true )
    state = State.new([], [ observer ], MockCdr )
    
    state.accept(cse1)
    assert_equal(0, observer.counter)
    
    state.accept(cse1)
    assert_equal(1, observer.counter)
    
    state.flush_failed(0)
    # still only one - nothing flushed
    assert_equal(1, observer.counter)
  end  
end
