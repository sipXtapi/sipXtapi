#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# set up the load path
$PROCESS_MANAGER_TEST_DIR = File.dirname(__FILE__)
$:.unshift File.join($PROCESS_MANAGER_TEST_DIR, "..", "lib")

# system requires
require 'test/unit'

# application requires
require 'utils'


class UtilsTest < Test::Unit::TestCase
  def test_get_process_pid
    # Kill off any existing "sleep" processes to avoid confusion
    # TODO: move the method kill_all_sleepers from process_manager_test into
    # a common test utils class and use it here.  Perhaps put Utils.get_process_pid
    # in a test utils class as well since it is really a test hack.
    `killall sleep`
    sleep(0.1)    # allow time for sleepers to die
    
    # No one should be sleeping now
    ps_out = `ps -fC sleep | grep -v defunct | grep sleep`
    puts ps_out
    assert_equal(0, ps_out.length)
    assert_nil(Utils.get_process_pid('sleep'))

    # Start a sleeper
    fork do
      exec('sleep 0.5')
    end

    # Now we have a sleeper
    pid = Utils.get_process_pid('sleep')
    assert_not_nil(pid, 'Expected a non-nil PID for the keyword "sleep"')
    ps_out = `ps -fC sleep | grep sleep`
    assert_not_nil(ps_out.index(pid.to_s))
    
    # Wait for the child process to finish so it will not become a zombie
    Process.wait
  end
end
