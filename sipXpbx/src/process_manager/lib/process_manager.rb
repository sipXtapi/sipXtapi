#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# system requires
require 'logger'

# application requires
require 'process_config'

=begin
:TODO: Start, stop, report status
:TODO: Don't start a process if it's already running
:TODO: Monitor processes and try to restart them if they go down

:TODO: Honor process config:
:TODO: "manage" flag -- if false then don't do anything with the process
:TODO: "stop" flag -- if false then have to kill the process to stop it
:TODO: "restart" flag -- if false then have to stop-start the process to restart it
:TODO: "max_restarts" setting
:TODO: "report", "max_reports" settings
:TODO: "failure_contact" setting
:TODO: "stdout", "stderr", "stdin" settings

:TODO: Remote service to replicate files
:TODO: Remote service to replicate sipdb data
:LATER: Create the process schema referenced by process config files
=end

# ProcessManager features:
# * Start, monitor, stop, report status on sipX processes
#   * Replaces watchdog, processcgi, WatchDog.xml, ProcessDefinitions.xml, ...
# * Offer network services:
#   * Replicate sipdb databases and arbitrary files (replace replicationcgi)
# * Configured via files that are installed into /etc/sipxpbx/process
class ProcessManager

  CONFIG_FILE_PATTERN = Regexp.new('.*\.xml')
  
public

  def initialize(config_dir)
    @config_dir = config_dir
    @config_map = init_process_config
    
    @log = Logger.new(STDOUT)
    @log.level = Logger::DEBUG
  end

  # Start the specified process.
  # If process_name is not given, then start all configured processes.
  def start(process_name = nil)
    # :NOW:
  end

private

  attr_reader :config_dir, :config_map, :log
  
  # Each config file in the config dir sets up a sipX process.
  # Read config info from those files and build a process map.
  def init_process_config
    config_map = {}
    get_process_config_files.each do |file|
      config = ProcessConfig.new(File.new(file))
      config_map[config.name] = config
    end
    config_map
  end
  
  # Return an array containing the paths of process config files.
  def get_process_config_files
    config_files = []
    Dir.foreach(@config_dir) do |file|
      config_files << File.join(@config_dir, file) if file =~ CONFIG_FILE_PATTERN
    end
    config_files
  end

  # Start the named process. Raise an exception if no such process is configured.
  def start_process_by_name(process_name)
    config = @config_map[process_name]
    if !config
      raise("Cannot start \"#{process_name}\", no such process is configured")
    end
    start_process(config)
  end

  # Start the named process. Raise an exception if no such process is configured.
  def start_process(process_config)
    # Get info on how to run the process. Assume that the config has been validated already.
    run = process_config.run
    command = run.command
    parameters = run.parameters
    defaultdir = run.defaultdir
    fork do
      log.debug("start_process: command = \"#{command}\", parameters = " +
                "\"#{parameters}\", defaultdir = \"#{defaultdir}\"")
      Dir.chdir(defaultdir) if defaultdir
      exec("#{command} #{parameters}")
    end
  end

end






