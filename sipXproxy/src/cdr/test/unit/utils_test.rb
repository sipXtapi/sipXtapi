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
    
    # another good input
    header = '<sip:100@pingtel.com;user=phone>'
    assert_equal('<sip:100@pingtel.com>', Utils.get_aor_from_header(header))
    
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

  def test_contact_without_params
    contact_with_no_params = '<sip:100@10.1.20.3:5100>'
    assert_equal(contact_with_no_params, Utils.contact_without_params(contact_with_no_params))
    
    contact_with_one_long_param = '<sip:100@10.1.20.3:5100;play=https%3A%2F%2Flocalhost%3A8091%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Dautoattendant%26name%3Daa_41>'
    assert_equal('<sip:100@10.1.20.3:5100>', Utils.contact_without_params(contact_with_one_long_param))
    
    contact_with_lots_of_params = '<sip:alice@example.com;one;more;for;you;nineteen;for;me>'
    assert_equal('<sip:alice@example.com>', Utils.contact_without_params(contact_with_lots_of_params))
    
    contact_with_no_greater_than_sign_at_end = 'sip:alice@example.com;one;more;for;you;nineteen;for;me'
    assert_equal('sip:alice@example.com', Utils.contact_without_params(contact_with_no_greater_than_sign_at_end))
  end

  def test_strip_v4_port
    assert_equal('1.1.1.1', SocketUtils.strip_v4_port('1.1.1.1:123'))
    assert_equal('1.1.1.1', SocketUtils.strip_v4_port('1.1.1.1'))
    assert_equal('domain.com', SocketUtils.strip_v4_port('domain.com'))
  end

end
