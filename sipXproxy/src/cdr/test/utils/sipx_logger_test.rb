#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'tempfile'
require 'test/unit'

$:.unshift File.join(File.dirname(__FILE__), '..', '..', 'lib')

require 'utils/sipx_logger'

class SipxLoggerTest < Test::Unit::TestCase
  
  def setup
    @tempfile = Tempfile.new("SipxLoggerTest")
    @log = SipxLogger.new(@tempfile) 
  end
  
  # Log a message.  The logged message should have the severity prefixed.
  def test_log
    @log.info('message in a keg')
    file = @tempfile.open
    assert_match(/".*":INFO:message in a keg\n/, file.readline)
  end
  
  # Log a message using a block.  The logged message should have the severity prefixed.
  def test_log_with_block
    @log.fatal {'message in a klein bottle'}
    file = @tempfile.open
    assert_match(/".*":CRIT:message in a klein bottle\n/, file.readline)
  end
  
end
