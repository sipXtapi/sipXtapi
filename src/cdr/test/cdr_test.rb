#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'test/unit'
require 'time'

$:.unshift File.join(File.dirname(__FILE__), "..", "lib")
require 'cdr'
require 'call_state_event'


class CallLegTest < Test::Unit::TestCase
  def test_has_duration?
    l = CallLeg.new('to')
    assert !l.has_duration?
    l.connect_time = 100
    assert !l.has_duration?
    l.end_time = 200
    assert l.has_duration?
    l.connect_time = nil
    assert !l.has_duration?    
  end
  
  def test_compare
    l1 = CallLeg.new('to1')
    l2 = CallLeg.new('to2')
    
    assert_raise ArgumentError do
      l1 <=> l2
    end
    
    l1.connect_time = l2.connect_time = 100
    l1.end_time = l2.end_time = 200
    l1.status = Cdr::CALL_COMPLETED_TERM
    l2.status = Cdr::CALL_FAILED_TERM
    
    assert l1 > l2
    
    l2.end_time = 250
    assert l1 > l2
    
    l2.status = Cdr::CALL_COMPLETED_TERM
    
    assert l1 < l2
    l1.end_time = 250
    
    assert l1 = l2
  end
end

def make_cse(args)
  cse = CallStateEvent.new
  # some defaults
  cse.event_seq= 1
  cse.event_time=Time.parse('1990-05-17T19:30:00.000Z')
  cse.event_type='R'
  cse.cseq=1
  cse.call_id='test'
  cse.from_tag=nil
  cse.to_tag=nil
  cse.from_url='sip:alice@example.com; tag=f'
  cse.to_url='sip:bob@example.com'
  cse.contact='sip:alice@1.1.1.1'
  
  if(args)
    args.each { |field, value| 
      setter = (field.to_s + "=").to_sym
      cse.send( setter, value ) 
    }
  end
  
  return cse 
end

class CallLegsTest < Test::Unit::TestCase
  
  def test_empty_done?
    assert !CallLegs.new.done?
  end
  
  def test_accept
    cl = CallLegs.new
    
    cse = make_cse(:event_type => 'S', :event_time => 100)
    assert_nil cl.accept_setup(cse)    
    assert_nil cl.best_leg    
    
    cse = make_cse(:event_type => 'F', :event_time => 200)
    leg = cl.accept_end(cse)    
    assert_kind_of(CallLeg, leg)
    
    assert_equal(100, leg.duration)
    
    assert_same(leg, cl.best_leg)
    
    assert cl.done? 
  end
  
  
  def test_accept_many
    cl = CallLegs.new
    
    cse = make_cse(:to_tag => 5, :event_type => 'S', :event_time => 100)
    assert_nil cl.accept_setup(cse)    
    assert_nil cl.best_leg    
    assert !cl.done? 
    
    cse = make_cse(:to_tag => 6, :event_type => 'S', :event_time => 101)
    assert_nil cl.accept_setup(cse)    
    assert_nil cl.best_leg    
    assert !cl.done? 
    
    cse = make_cse(:to_tag => 6, :event_type => 'F', :event_time => 200)
    leg = cl.accept_end(cse)    
    assert_kind_of(CallLeg, leg)
    assert_same(leg, cl.best_leg)
    assert !cl.done? 
    
    cse = make_cse(:to_tag => 5, :event_type => 'E', :event_time => 201)
    leg = cl.accept_end(cse)    
    assert_kind_of(CallLeg, leg)
    
    assert_equal(101, leg.duration)
    
    assert_same(leg, cl.best_leg)
    
    assert cl.done? 
  end
  
  def test_accept_out_of_order
    cl = CallLegs.new
    
    cse = make_cse(:to_tag => 6, :event_type => 'F', :event_time => 200)
    leg = cl.accept_end(cse)    
    assert_nil cl.best_leg    
    assert !cl.done? 
    
    cse = make_cse(:to_tag => 5, :event_type => 'S', :event_time => 101)
    assert_nil cl.accept_setup(cse)    
    assert_nil cl.best_leg    
    assert !cl.done? 
    
    cse = make_cse(:to_tag => 6, :event_type => 'S', :event_time => 100)
    leg = cl.accept_setup(cse)    
    assert_kind_of(CallLeg, leg)
    assert_same(leg, cl.best_leg)
    assert !cl.done? 
    
    cse = make_cse(:to_tag => 5, :event_type => 'E', :event_time => 201)
    leg = cl.accept_end(cse)    
    assert_kind_of(CallLeg, leg)
    
    assert_equal(100, leg.duration)
    
    assert_same(leg, cl.best_leg)
    
    assert cl.done? 
  end
  
  
end


class CdrTest < Test::Unit::TestCase
  def test_accept_request
    cdr = Cdr.new('test')
    cse = make_cse(:to_tag => nil, :event_type => 'R', :event_time => 100)
    assert_nil cdr.accept(cse)    
  end
  
  def test_accept_setup
    cdr = Cdr.new('test')
    cse = make_cse(:to_tag => nil, :event_type => 'S', :event_time => 100)
    assert_nil cdr.accept(cse)    
  end
  
  def test_accept_failure
    cdr = Cdr.new('test')
    cse = make_cse(:to_tag => 6, :event_type => 'F', :event_time => 100)
    assert_nil cdr.accept(cse)    
  end
  
  
  def test_accept_end
    cdr = Cdr.new('test')
    cse = make_cse(:to_tag => 6, :event_type => 'E', :event_time => 100)
    assert_nil cdr.accept(cse)    
  end
  
  
  def test_accept_transfer
    cdr = Cdr.new('test')
    cse = make_cse(:to_tag => 6, :event_type => 'T', :event_time => 100)
    assert_nil cdr.accept(cse)    
  end
  
end
