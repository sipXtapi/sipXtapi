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
require 'net/http'
require 'soap/rpc/driver'

# application requires
require 'process_manager'
require 'process_manager_server'


class ProcessManagerServerTest < Test::Unit::TestCase

  TEST_PORT = 2000

  def start_server
    # not starting each time, port doesn't get freed in time
    return if defined? @@server

    puts "Starting"
    process_config_dir = File.join($PROCESS_MANAGER_TEST_DIR, "data", "process")
    pm = ProcessManager.new(:ProcessConfigDir => process_config_dir)
    
    # override the PID dir to avoid complaints about the directory not existing
    pm.pid_dir = '/tmp'
    
    httpd = ProcessManagerServer.new(pm, :Port => TEST_PORT)

    trap(:INT) do
      httpd.shutdown
    end

    @@server = Thread.new do
      httpd.start
    end
    puts "Started"
  end
  
  def setup
    super
    start_server
    @pm = SOAP::RPC::Driver.new("http://localhost:#{TEST_PORT}",
                                 ProcessManagerServer::SOAP_NAMESPACE)
    @pm.wiredump_dev = STDERR if $DEBUG
    @pm.add_method('manageProcesses', 'input')
    @pm.add_method('getProcessStatus')
  end
  
  def teardown
  end

  def test_manageProcesses
    input = ProcessManagerServer::ManageProcessesInput.new
    input.processes = ProcessManagerServer::Array['sleeper']
    input.verb = 'start'
    @pm.manageProcesses(input)
  end

  def test_getProcessStatus
    result_array = @pm.getProcessStatus()
    s1 = result_array[0]
    puts "s1: name=#{s1.name} status=#{s1.status}"
    s2 = result_array[1]
    puts "s2: name=#{s2.name} status=#{s2.status}"
  end

end
