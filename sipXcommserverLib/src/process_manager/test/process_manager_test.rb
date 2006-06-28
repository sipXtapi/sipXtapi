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
require 'tmpdir'
require 'test/unit'

# application requires
require 'process_manager'
require 'utils'


class ProcessManagerTest < Test::Unit::TestCase
  SLEEP_CMD = 'sleep'
  TEST_PROCESS_NAME = 'sleeper'
  TEST_PROCESS_RUN_PARAMS = "10"
  
  # If set, then this becomes a prefix to the default sipX directories
  SIPX_PREFIX = 'SIPX_PREFIX'
  
  def setup
    @process_config_dir = File.join($PROCESS_MANAGER_TEST_DIR, "data", "process")
    @pm = ProcessManager.new(:ProcessConfigDir => @process_config_dir)
    @config_file = File.join(@process_config_dir, 'sleeper.xml')
    @tmpdir = Dir.tmpdir
    assert(@tmpdir, "There is no tmpdir, can't run tests")
        
    # Override the PID dir to avoid complaints about the directory not existing
    @pm.pid_dir = '/tmp'    
  end
  
  def test_create_pm
    assert(@pm)
    assert_equal(@process_config_dir, @pm.send(:process_config_dir))
    assert(File.exists?(@config_file))
  end
  
  def test_init_process_config
    process_config_map = @pm.send(:process_config_map)
    assert(process_config_map)
    assert_equal(1, process_config_map.size)
    config = process_config_map[TEST_PROCESS_NAME]
    assert(config)
    assert_equal(TEST_PROCESS_NAME, config.name)
    assert_equal(TEST_PROCESS_RUN_PARAMS, config.run.parameters)
  end
  
  def test_get_process_config_files
    config_files = @pm.send(:get_process_config_files)
    assert_equal(1, config_files.length)
    assert_equal(@config_file, config_files[0])
  end
  
  def test_init_pid_dir
    # If SIPX_PREFIX is not set, then the pid_dir must be the default
    ENV[ProcessManager::SIPX_PREFIX] = nil
    @pm.send(:init_pid_dir)
    assert_equal(ProcessManager::PID_DIR_DEFAULT, @pm.send(:pid_dir))
    
    # If SIPX_PREFIX is set, then the pid_dir must be the default with
    # SIPX_PREFIX prepended
    prefix = '/tmp'
    ENV[ProcessManager::SIPX_PREFIX] = prefix
    @pm.send(:init_pid_dir)
    assert_equal(File.join(prefix, ProcessManager::PID_DIR_DEFAULT), @pm.send(:pid_dir))    
  end
  
  def test_start_process
    config = ProcessConfig.new
  
    # Run the echo command.  Print out "moose" with no newline.
    config.name = 'moose'
    run = ProcessConfig::Run.new("echo",              # command
                                 "-n moose")          # parameters
    config.run = run
    @pm.start_process(config)
  
    # Create a temporary file to check that the defaultdir setting is right
    tempfilename = 'process_manager_test_tempfile'
    tempfilepath = File.join(@tmpdir, tempfilename)
    
    # Delete the temp file if it already exists so we can do a clean test
    if File.exists?(tempfilepath)
      File.delete(tempfilepath)
    end
    
    # Run the command
    run = ProcessConfig::Run.new('touch',           # command
                                 tempfilename,      # parameters
                                 @tmpdir)           # defaultdir
    config.name = 'touch'
    config.run = run
    assert(!File.exists?(tempfilepath), "File \"#{tempfilepath}\" must not exist but does")
    @pm.start_process(config)
    
    # Give the child process some time to create the file
    1.upto(100) do |i|
      if File.exists?(tempfilepath)
        break
      end
      sleep 0.01                                    # sleep for 1/100 of a second
    end
    
    # Verify that it created the file
    assert(File.exists?(tempfilepath), "File \"#{tempfilepath}\" must exist but doesn't")
    
    # Clean up
    File.delete(tempfilepath)
    
    # Verify that we won't start a named process again if it's already running
    run = ProcessConfig::Run.new(SLEEP_CMD,         # command
                                 1)                 # parameters
    config.name = SLEEP_CMD
    config.run = run
    pid1 = @pm.start_process(config)
    pid2 = @pm.start_process(config)
    assert_equal(pid1, pid2, 'Expected the same PID but got two different PIDs')
  end

  def test_stop_process_by_name
    # Kill off any existing "sleep" processes to avoid confusion
    kill_all_sleepers

    config = ProcessConfig.new
    run = ProcessConfig::Run.new(SLEEP_CMD,         # command
                                 999)               # parameters
    config.name = 'another_sleep'  # don't use the same name as the previous test
    config.run = run
    
    # Start the process
    pid = @pm.start_process(config)
    assert_equal(Utils.get_process_pid(SLEEP_CMD), pid)
    
    # Stop the process, should work
    did_stop = @pm.stop_process_by_name(config.name)
    assert_equal(true, did_stop)
    puts `ps -fC #{SLEEP_CMD}`
    assert_nil(Utils.get_process_pid(SLEEP_CMD))
    
    # Ask to stop it again, should already be stopped.
    did_stop = @pm.stop_process_by_name(config.name)
    assert_equal(false, did_stop)
  end

  def test_restart_process
    # Kill off any existing "sleep" processes to avoid confusion
    kill_all_sleepers
    
    config = ProcessConfig.new
    run = ProcessConfig::Run.new(SLEEP_CMD,         # command
                                 999)               # parameters
    config.name = 'yet_another_sleep'  # don't use the same name as the previous test
    config.run = run
    
    # Start the process
    pid1 = @pm.start_process(config)
    assert_equal(Utils.get_process_pid(SLEEP_CMD), pid1)
    
    # Test restarting the process
    pid2 = @pm.restart_process(config)
    sleep(0.1)   # give the killed process time to die
    assert_equal(Utils.get_process_pid(SLEEP_CMD), pid2)
    assert_not_equal(pid1, pid2)
    
    # Stop the restarted process so we don't leave debris behind
    @pm.stop_process_by_name(config.name)
  end

  def test_create_process_pid_file
    process_name = 'extradition'
    pid = 1959
    
    # Change the PID dir to be temporary, for testing
    @pm.send(:pid_dir=, @tmpdir)
    
    # Make sure the PID file doesn't already exist, for a clean test
    pid_file_path = File.join(@tmpdir, process_name + ProcessManager::PID_FILE_EXT)
    if File.exists?(pid_file_path)
      File.delete(pid_file_path)
    end
    assert(!File.exists?(pid_file_path), "File \"#{pid_file_path}\" must not exist but does")
    
    # Create the PID file
    path = @pm.send(:create_process_pid_file, process_name, pid)
    assert_equal(pid_file_path, path, "Expected PID file path to be \"#{pid_file_path}\" but was \"#{path}\"")

    # Check that it worked
    assert(File.exists?(pid_file_path), "File \"#{pid_file_path}\" must exist but doesn't")
    File.open(pid_file_path) do |file|
      saved_pid = file.readline
      assert_equal(pid, saved_pid.to_i)
    end
    
    # Clean up
    File.delete(pid_file_path)
  end  

  def test_get_sipx_directory
    begin
      # Use an unknown sipX directory name, should blow up
      assert_raises(@pm.get_sipx_directory('unknown'))
    rescue RuntimeError
    end
    
    # Try a known sipX directory name, with or without SIPX_PREFIX
    save_prefix = ENV[SIPX_PREFIX]
    begin
      ENV[SIPX_PREFIX] = '/my_prefix'
      sipx_dir = @pm.get_sipx_directory(ProcessManager::DATA_DIR)
      assert_equal('/my_prefix/var/sipxdata/', sipx_dir)
      
      ENV[SIPX_PREFIX] = nil
      sipx_dir = @pm.get_sipx_directory(ProcessManager::DATA_DIR)
      assert_equal('/var/sipxdata/', sipx_dir)
    ensure
      ENV[SIPX_PREFIX] = save_prefix
    end
  end

  def test_prepend_sipx_prefix
    save_prefix = ENV[SIPX_PREFIX]
    begin
      ENV[SIPX_PREFIX] = '/my_prefix'
      prefixed_dir = @pm.send(:prepend_sipx_prefix, 'banana')
      assert_equal('/my_prefix/banana', prefixed_dir)

      ENV[SIPX_PREFIX] = nil
      prefixed_dir = @pm.send(:prepend_sipx_prefix, 'banana')
      assert_equal('banana', prefixed_dir)      
    ensure
      ENV[SIPX_PREFIX] = save_prefix
    end
  end

  def kill_all_sleepers
    # Kill off any existing "sleep" processes to avoid confusion
    `killall #{SLEEP_CMD}`
    
    # Sleep for a little while to give the processes time to die
    sleep(0.1)
  end
end
