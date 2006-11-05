#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'test/unit'
require 'thread'

$:.unshift(File.join(File.dirname(__FILE__), '..', '..', 'lib'))

require 'db/cdr_writer'

class CdrWriterTest < Test::Unit::TestCase
  
  def test_insert_sql
    sql = CdrWriter.insert_sql
    assert_equal("INSERT INTO cdrs ( call_id, from_tag, to_tag, caller_aor, callee_aor, " +
      "start_time, connect_time, end_time, " +
      "termination, failure_status, failure_reason, " +
      "call_direction ) " +
      "VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )", sql)
  end
end
