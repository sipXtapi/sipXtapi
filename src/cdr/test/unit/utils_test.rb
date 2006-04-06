#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# set up the load path
$thisdir = File.dirname(__FILE__)
$:.unshift($thisdir)
$:.unshift(File.join($thisdir, ".."))
$:.unshift(File.join($thisdir, "..", ".."))

# application requires
require 'test_helper'
require 'utils'
require 'exceptions'


class UtilsTest < Test::Unit::TestCase

  def test_get_get_aor_from_header
    # test get_aor_from_header with a good input
    header = 'From: Alice <sip:alice@atlanta.com>;tag=1928301774'
    assert_equal('From: Alice <sip:alice@atlanta.com>',
                 Utils.get_aor_from_header(header))
    
    # Test get_aor_from_header with a bad input: there is no tag.
    # Must raise an exception.
    assert_raise(BadSipHeaderException) do
      Utils.get_aor_from_header('From: Alice <sip:alice@atlanta.com>')
    end
    
    # Test get_aor_from_header with a bad input: there is no tag.
    # Must not raise an exception because we tell it that a tag is not required.
    assert_nothing_thrown do
      Utils.get_aor_from_header('From: Alice <sip:alice@atlanta.com>', false)
    end
  end
  
  def test_contact_host
    contacts = ["\"Jorma Kaukonen\"<sip:187@10.1.1.170:1234>;tag=1c32681",
                "<sip:187@10.1.1.170:1234>",
                "sip:187@10.1.1.170:1234",
                "sip:187@10.1.1.170"]
    contacts.each do |contact|
      assert_equal("10.1.1.170", Utils.contact_host(contact))
    end
  end

  def test_remove_part_of_str_beginning_with_char
    str = "a.b.c"
    assert_equal("a", Utils.remove_part_of_str_beginning_with_char(str, "."))
    assert_equal(str, Utils.remove_part_of_str_beginning_with_char(str, "^"))    
  end

end
