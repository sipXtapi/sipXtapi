#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# Application requires.  Assume that the load path has been set up for us.
require 'configure'
require 'database_url'
require 'exceptions'


class CallResolverConfigure

  # Default config file path
  DEFAULT_CONFIG_FILE = '/etc/sipxpbx/callresolver-config'
  
  # If set, then this becomes a prefix to the default config file path
  SIPX_PREFIX = 'SIPX_PREFIX'

  # If the daily run is enabled, then it happens at 4 AM, always
  DAILY_RUN_TIME = '04:00'

  LOCALHOST = 'localhost'

  # How many seconds are there in a day
  SECONDS_IN_A_DAY = 86400

  # Configuration parameters and defaults

  # Whether console logging is enabled or disabled.  Legal values are "ENABLE"
  # or "DISABLE".  Comparison is case-insensitive with this and other values.
  LOG_CONSOLE_CONFIG = 'SIP_CALLRESOLVER_LOG_CONSOLE'
  LOG_CONSOLE_CONFIG_DEFAULT = Configure::DISABLE
  
  # The directory holding log files.  The default value is prefixed by
  # $SIPX_PREFIX if that environment variable is defined.
  LOG_DIR_CONFIG = 'SIP_CALLRESOLVER_LOG_DIR'
  LOG_DIR_CONFIG_DEFAULT = '/var/log/sipxpbx'
  
  # Logging severity level
  LOG_LEVEL_CONFIG = 'SIP_CALLRESOLVER_LOG_LEVEL'
  LOG_LEVEL_CONFIG_DEFAULT = 'NOTICE'
  
  LOG_FILE_NAME = 'sipcallresolver.log'

  DAILY_RUN = 'SIP_CALLRESOLVER_DAILY_RUN'
  DAILY_RUN_DEFAULT = Configure::DISABLE
  
  PURGE = 'SIP_CALLRESOLVER_PURGE'
  PURGE_DEFAULT = Configure::ENABLE
  
  PURGE_AGE_CDR = 'SIP_CALLRESOLVER_PURGE_AGE_CDR'
  PURGE_AGE_CDR_DEFAULT = '35'
  
  PURGE_AGE_CSE = 'SIP_CALLRESOLVER_PURGE_AGE_CSE'
  PURGE_AGE_CSE_DEFAULT = '7'
  
  CSE_HOSTS = 'SIP_CALLRESOLVER_CSE_HOSTS'
  CSE_HOSTS_DEFAULT = "#{LOCALHOST}:#{DatabaseUrl::DATABASE_PORT_DEFAULT}"

  
  # Map from the name of a log level to a Logger level value.
  # Map the names of sipX log levels (DEBUG, INFO, NOTICE, WARNING, ERR, CRIT,
  # ALERT, EMERG) and Logger log levels (DEBUG, INFO, WARN, ERROR, FATAL) into
  # Logger log levels.
  LOG_LEVEL_MAP = {
    "DEBUG"   => Logger::DEBUG, 
    "INFO"    => Logger::INFO, 
    "NOTICE"  => Logger::INFO, 
    "WARN"    => Logger::WARN,
    "WARNING" => Logger::WARN,
    "ERR"     => Logger::ERROR, 
    "CRIT"    => Logger::FATAL,
    "ALERT"   => Logger::FATAL,
    "EMERG"   => Logger::FATAL
  }

  # Specify this string as the config_file to get a completely default config 
  DEFAULT_CONFIG = 'default_config'

  #-----------------------------------------------------------------------------
  # Public methods

public

  def initialize(config_file = nil)
    # If the config_file arg is nil, then find the config file in the default location
    @config_file = apply_config_file_default(config_file)

    # Set up a generic Configure object that just knows how to parse a config
    # file with param:value lines.
    if @config_file == DEFAULT_CONFIG
      @config = Configure.new()
    else
      if File.exists?(@config_file)
        @config = Configure.new(@config_file)
      else
        puts("Config file #{@config_file} not found, using default settings")
        @config = Configure.new()
      end
    end
    
    # Read logging config and initialize logging.  Do this before initializing
    # the rest of the config so we can use logging there.
    init_logging
    
    # Finish setting up the config
    finish_config
  end

  #-----------------------------------------------------------------------------
  # Public configuration

  BOOL_PARAMS = [:daily_run?, :ha?, :purge?]
  OTHER_PARAMS = [:cdr_database_url, :cse_database_urls, :config_file,
                  :daily_start_time, :daily_end_time,
                  :host_list, :host_url_list, :host_port_list,
                  :log, :log_device,
                  :purge_start_time_cdr, :purge_start_time_cse]
  ALL_PARAMS = BOOL_PARAMS + OTHER_PARAMS

  # Return true if daily runs of the call resolver are enabled, false otherwise
  def daily_run?
    @daily_run
  end

  # Return true if High Availability (HA) is enabled, false otherwise
  def ha?
    @ha
  end

  # Return true if database purging is enabled, false otherwise
  def purge?
    @purge
  end

  # Define a reader for each of the other params
  OTHER_PARAMS.each {|sym| attr_reader(sym)}

  # Access the config as an array.  Use this method *only* for plugin config
  # params that are unknown to the call resolver.  All known params should be
  # retrieved using the above accessors.
  def [](param)
    config[param]
  end

  #-----------------------------------------------------------------------------
  # Private methods

private

  attr_accessor :config
    
  # For testing purposes only.  Test code calls these methods using low-level
  # message sending.
  
  attr_writer :cse_database_urls
  
  def host_port_list=(host_port_list)
    @host_port_list = host_port_list
    
    # Recompute the CSE DB urls, since the host port list changed
    set_cse_database_urls_config(config)
  end

  #-----------------------------------------------------------------------------
  # Logging configuration
    
  # Set up logging.  Return the Logger.
  def init_logging
    @log_device = nil
    @log = nil

    # Read the logging config
    set_log_console_config(@config)
    set_log_dir_config(@config)
    set_log_level_config(@config)

    # If console logging was specified, then do that.  Otherwise log to a file.
    if @log_console
      @log_device = STDOUT
    else
      if File.exists?(@log_dir)
        log_file = File.join(@log_dir, LOG_FILE_NAME)
        @log_device = log_file
        # If the file exists, then it must be writable. If it doesn't exist,
        # then the directory must be writable.
        if File.exists?(log_file)
          if !File.writable?(log_file)
            puts("init_logging: Log file \"#{log_file}\" exists but is not writable. " +
                 "Log messages will go to the console.")
            @log_device = STDOUT
          end
        else
          if !File.writable?(@log_dir)
            puts("init_logging: Log directory \"#{@log_dir}\" is not writable. " +
                 "Log messages will go to the console.")
            @log_device = STDOUT
          end
        end
      else
        puts("Unable to open log file, log directory \"#{@log_dir}\" does not " +
             "exist.  Log messages will go to the console.")
        @log_device = STDOUT
      end
    end
    @log = Logger.new(@log_device)

    # Set the log level from the configuration
    @log.level = @log_level

    # Override the log level to DEBUG if $DEBUG is set.
    # :TODO: figure out why this isn't working.
    if $DEBUG then
      @log.level = Logger::DEBUG
    end
    
    @log
  end
  
  # Set the console logging from the configuration.
  # Return the console logging boolean.
  def set_log_console_config(config)
    # Look up the config param
    @log_console = config[LOG_CONSOLE_CONFIG]
    
    # Apply the default if the param was not specified
    @log_console ||= LOG_CONSOLE_CONFIG_DEFAULT

    # Convert to a boolean
    if @log_console.casecmp(Configure::ENABLE) == 0
      @log_console = true
    elsif @log_console.casecmp(Configure::DISABLE) == 0
      @log_console = false
    else
      raise(ConfigException, "Unrecognized value \"#{@log_console}\" for " +
            "#{LOG_CONSOLE_CONFIG}.  Must be ENABLE or DISABLE.")
    end
  end
  
  # Set the log directory from the configuration.  Return the log directory.
  def set_log_dir_config(config)
    # Look up the config param
    @log_dir = config[LOG_DIR_CONFIG]
    
    # Apply the default if the param was not specified
    if !@log_dir
      @log_dir = LOG_DIR_CONFIG_DEFAULT
      
      # Prepend the prefix dir if $SIPX_PREFIX is defined
      prefix = ENV[SIPX_PREFIX]
      if prefix
        @log_dir = File.join(prefix, @log_dir)
      end      
    end
    
    @log_dir
  end
  
  # Set the log level from the configuration.  Return the log level.
  def set_log_level_config(config)
    # Look up the config param
    log_level_name = config[LOG_LEVEL_CONFIG]
    
    # Apply the default if the param was not specified
    log_level_name ||= LOG_LEVEL_CONFIG_DEFAULT
    
    # Convert the log level name to a Logger log level
    @log_level = log_level_from_name(log_level_name)
    
    # If we don't recognize the name, then refuse to run.  Would be nice to
    # log a warning and continue, but there is no log yet!
    if !@log_level
      raise(CallResolverException, "Unknown log level: #{log_level_name}")   
    end
    
    @log_level
  end

  # Given the name of a log level, return the log level value, or nil if the
  # name is not recognized.
  # We accept sipX log levels and Logger log levels and map them into Logger log
  # levels.
  def log_level_from_name(name)
    LOG_LEVEL_MAP[name]
  end

  #-----------------------------------------------------------------------------
  
  # Finish setting up the config.  Logging has already been set up before this,
  # so we can log messages in the methods that are called here.
  def finish_config
    # Read config params, applying defaults
    
    set_cdr_database_url_config(@config)
    
    # These two methods must get called in this order
    set_cse_hosts_config(@config)
    set_cse_database_urls_config(@config)
    
    set_daily_run_config(@config)
    set_daily_start_time_config(@config)
    set_purge_config(@config)
    set_purge_start_time_cdr_config(@config)
    set_purge_start_time_cse_config(@config)
  end
  
  # Given a config_file name, if it is non-nil then just return it.
  # If it's nil then return the default config file path, prepending
  # $SIPX_PREFIX if that has been set.
  def apply_config_file_default(config_file)
    if !config_file
      config_file = DEFAULT_CONFIG_FILE
      prefix = ENV[SIPX_PREFIX]
      if prefix
        config_file = File.join(prefix, config_file)
      end
    end
    
    config_file
  end

  def set_cdr_database_url_config(config)
    # :TODO: read CDR database URL params from the Call Resolver config file
    # rather than just hardwiring default values.
    @cdr_database_url = DatabaseUrl.new
  end
  
  # Return an array of CSE database URLs.  With an HA configuration, there are
  # multiple CSE databases.  Note that usually one of these URLs is identical
  # to the CDR database URL, since a standard master server runs both the
  # proxies and the call resolver, which share the SIPXCDR database.
  def set_cse_database_urls_config(config)
    @cse_database_urls = []
    if @host_port_list and (@host_port_list.size > 0)
      # Build the list of CSE DB URLs.  From Call Resolver's point of view,
      # each URL is 'localhost:<port>'.  Stunnel takes care of forwarding the
      # local port to the database on a remote host.
      @host_port_list.each do |port|
        url = DatabaseUrl.new(DatabaseUrl::DATABASE_DEFAULT, port)
        @cse_database_urls << url
      end
    else
      @cse_database_urls << cdr_database_url
    end
    
    @cse_database_urls
  end
  
  # Enable/disable the daily run from the configuration.
  # Return true if daily runs are enabled, false otherwise.
  def set_daily_run_config(config)
    # Look up the config param
    @daily_run = config[DAILY_RUN]
    
    # Apply the default if the param was not specified
    @daily_run ||= DAILY_RUN_DEFAULT

    # Convert to a boolean
    if @daily_run.casecmp(Configure::ENABLE) == 0
      @daily_run = true
    elsif @daily_run.casecmp(Configure::DISABLE) == 0
      @daily_run = false
    else
      raise(ConfigException, "Unrecognized value \"#{@daily_run}\" for " +
            "#{DAILY_RUN}.  Must be ENABLE or DISABLE.")
    end
  end

  # Compute the start time of the daily call resolver run.
  # We decided not to make this configurable.  Too complicated given that the
  # cron job always runs at a fixed time.
  def set_daily_start_time_config(config)
    # Always start the time window at the time the resolver runs
    daily_start = DAILY_RUN_TIME
    
    # Turn the start time into a date/time.
    # Get today's date, cut out the date and paste our start time into
    # a time string.
    today = Time.now
    todayString = today.strftime("%m/%d/%YT")
    startString = todayString + daily_start
    
    # Convert to time, start same time yesterday
    @daily_start_time = Time.parse(startString)
    #log.debug("set_daily_start_time_config: String #{startString}, time #{@daily_start_time}")    
    @daily_end_time = @daily_start_time
    @daily_start_time -= SECONDS_IN_A_DAY   # 24 hours
  end
  
  # Enable/disable the daily run from the configuration.
  # Return true if purging is enabled, false otherwise.
  def set_purge_config(config)
    # Look up the config param
    @purge = config[PURGE]
    
    # Apply the default if the param was not specified
    @purge ||= PURGE_DEFAULT

    # Convert to a boolean
    if @purge.casecmp(Configure::ENABLE) == 0
      @purge = true
    elsif @purge.casecmp(Configure::DISABLE) == 0
      @purge = false
    else
      raise(ConfigException, "Unrecognized value \"#{@purge}\" for " +
            "#{PURGE}.  Must be ENABLE or DISABLE.")
    end
  end  
  
  # Compute start time of CDR records to be purged from configuration
  def set_purge_start_time_cdr_config(config)
    # Look up the config param
    purge_age = config[PURGE_AGE_CDR]
    
    # Apply the default if the param was not specified
    purge_age ||= PURGE_AGE_CDR_DEFAULT
    
    # Convert to number
    purge_age = purge_age.to_i
    
    if (purge_age <= 0)
      raise(ConfigException, "Illegal value \"#{@purge_age}\" for " +
            "#{PURGE_AGE_CDR}.  Must be a number greater than 0.")
    end
    # Get today's date
    today = Time.now
    @purge_start_time_cdr = today - (SECONDS_IN_A_DAY * purge_age)
  end    
    
  # Compute start time of CSE records to be purged from configuration
  def set_purge_start_time_cse_config(config)
    # Look up the config param
    purge_age = config[PURGE_AGE_CSE]
    
    # Apply the default if the param was not specified
    purge_age ||= PURGE_AGE_CSE_DEFAULT
    
    # Convert to number
    purge_age = purge_age.to_i
    
    if (purge_age <= 0)
      raise(ConfigException, "Illegal value \"#{@purge_age}\" for " +
            "#{PURGE_AGE_CSE}.  Must be a number greater than 0.")
    end    
                
    # Get today's date
    today = Time.now
    @purge_start_time_cse = today - (SECONDS_IN_A_DAY * purge_age)
  end

  # Get distributed CSE hosts from the configuration. Initialize @host_url_list
  # to a list of hostnames and @host_port_list to the corresponding list of
  # ports. Call resolver connects to each of these ports on 'localhost' via the
  # magic of stunnel, so it doesn't ever use the hostnames.
  def set_cse_hosts_config(config)
    @host_list = config[CSE_HOSTS]    
    @host_list ||= CSE_HOSTS_DEFAULT
    
    @host_url_list = []
    @host_port_list = []
    @ha = false
    # Split host list into separate host:port names, then build two
    # arrays of URLs and ports.
    host_array = @host_list.split(',')
    host_array.each do |host_string|
      host_elements = host_string.split(':')
      # Strip leading and trailing whitespace
      host_elements[0] = host_elements[0].strip
      # Test if port was specified      
      if host_elements.length == 1
        # Supply default port for localhost
        if host_elements[0] == LOCALHOST
          host_elements[1] = DatabaseUrl::DATABASE_PORT_DEFAULT
        else
          Utils.raise_exception(
            "No port specified for host \"#{host_elements[0]}\". " +
            "A port number for hosts other than  \"localhost\" must be specified.",
            ConfigException)
        end
      else
        # Strip whitespace from port
        host_elements[1] = host_elements[1].strip
      end
      @host_url_list << host_elements[0]
      host_port = host_elements[1].to_i
      if host_port == 0
        raise(ConfigException, "Port for #{host_elements[0]} is invalid.")
      end
      @host_port_list << host_port
      log.debug("set_cse_hosts_config: host name #{host_elements[0]}, host port: #{host_elements[1]}")
      # If at least one of the hosts != 'localhost' we are HA enabled
      if host_elements[0] != 'localhost' && ! @ha
        @ha = true
        log.debug("get_cse_host: Found host other than localhost - enable HA")
      end
    end
    @host_port_list
  end  
  
end
