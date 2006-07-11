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

# System requires

# Load ActiveRecord so we'll test with the changes it makes to Logger
require 'rubygems'            # Ruby packaging and installation framework
require_gem 'activerecord'    # object-relational mapping layer for Rails
require 'tempfile'
require 'test_helper'

# Application requires.  Assume that the load path has been set up for us.
require 'sipx_logger'


class SipxLoggerTest < Test::Unit::TestCase
  
  def setup
    @tempfile = Tempfile.new("SipxLoggerTest")
    @log = SipxLogger.new(@tempfile)    
  end
  
  # Log a message.  The logged message should have the severity prefixed.
  def test_log
    msg = 'message in a keg'
    @log.info(msg)
    file = @tempfile.open
    assert_equal(SipxLogger::SIPX_INFO + ": " + msg + "\n", file.readline)
  end
  
  # Log a message using a block.  The logged message should have the severity prefixed.
  def test_log_with_block
    msg = 'message in a klein bottle'
    @log.fatal {msg}
    file = @tempfile.open
    assert_equal(SipxLogger::SIPX_CRIT + ": " + msg + "\n", file.readline)
  end
  
end
