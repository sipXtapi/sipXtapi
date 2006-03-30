#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# The Configure class reads in config params from the standard sipX config file,
# call-resolver-config.  (Don't use the name "Config" for this class because
# "Config" is the name of an existing Ruby module.)
# There is also a config subdirectory of this project.  That contains Rails
# code that is only used for unit testing.

# system requires
require 'logger'              # writes log messages to a file or stream

# application requires
require 'exceptions'


class Configure

public

  # Initialize inputs:
  #   config_file - relative or absolute path to config file (String)
  #   log - Logger instance to log to
  def initialize(config_file = nil, log = nil)
    @config_file = config_file ? config_file : CONFIG_FILE
    
    if !log
      @log = Logger.new(STDOUT)
      @log.level = Logger::ERROR
    end
    
    # create a Hash to hold the param name to value map
    @map = Hash.new
    
    # parse the config file and fill in the map
    parse_config(@config_file)
  end

  # Return the value for the named parameter, or nil if no such parameter exists
  def [](param_name)
    return @map[param_name]
  end

private

  # Config file syntax
  COMMENT_CHAR = ?#
  SEPARATOR_CHAR = ':'    # separate param names from values
  
  CONFIG_FILE = '../../etc/sipxpbx/callresolver-config'
  
  # Parse the config file and construct a hash mapping param names to values.
  def parse_config(config_file)
    begin
      File.open(config_file, 'r') do |file|
        if @log.debug?
          @log.debug("Parsing config file #{config_file}")  
        end
        
        line_num = 1
        file.each_line do |line|
          line.strip!   # trim leading and trailing whitespace
          if line[0] != COMMENT_CHAR    # ignore comment lines
            # discard any trailing comments
            comment = line.index(COMMENT_CHAR)
            if comment
              line = line[0...comment]
            end
            
            # split the line into name and value
            sep = line.index(SEPARATOR_CHAR)
            if !sep
              msg = "Missing \"#{SEPARATOR_CHAR}\" on line #{line_num}: #{line}"
              @log.error(msg)
              raise(ConfigException, msg)
            end
            name = line[0...sep]
            name.strip!
            value = line[(sep + 1)..-1]
            value.strip!
            if @log.debug?
              @log.debug("Name = #{name}, value=#{value}, line number #{line_num}")
            end
            @map[name] = value
          end
          line_num += 1
        end
      end
    rescue SystemCallError
      @log.error("Error opening \"#{config_file}\": #{$!}")
      raise   # unrecoverable, so rethrow
    end
  end

end

