#!/usr/bin/env ruby

# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.

DIR = File.dirname(__FILE__)
$:.unshift File.join(DIR, "..")
require 'test/unit'
require 'stringio'
load 'sipdb-dump'

class SipdbDumpTest < Test::Unit::TestCase

  def test_dump
      out = StringIO.new
      dumper = SipDbDumper.new(out)
      dumper.dump("#{DIR}/registration.xml")
      expected = ["callid=c29c2b63-de009a78-44188331@10.1.1.161",
       "cseq=958",
       "primary=sipxchange.pingtel.com",
       "uri=sip:160@pingtel.com",
       "contact=&lt;sip:160@10.1.1.161;transport=tcp&gt;",
       "contact_host=10.1.1.161",
       "qvalue=",
       "expires=1164082496",
       "expired=true",
       "instance_id=",
       "gruu=",
       "update_number=0x456232bf00000287"]
      assert_equal expected, out.string.strip.split('\n')
  end
  
  def test_contact_host
      out = StringIO.new
      SipDbDumper.new().dump_contact(out, "contact", "&quot;D K&quot;&lt;sip:154@10.1.1.164;LINEID=7bfd&gt;")
      assert_equal 'contact=&quot;D K&quot;&lt;sip:154@10.1.1.164;LINEID=7bfd&gt;\ncontact_host=10.1.1.164\n', out.string
  end
  
  def test_expires
      out = StringIO.new
      SipDbDumper.new().dump_expires(out, "expires", "1")
      assert_equal 'expires=1\nexpired=true\n', out.string    

      out2 = StringIO.new
      future = Time.now.to_i + 100
      SipDbDumper.new().dump_expires(out2, "expires", future)
      assert_equal "expires=#{future}\\nexpired=false\\n", out2.string    
  end
  
end


