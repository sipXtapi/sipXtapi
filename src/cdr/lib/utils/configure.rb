#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# The Configure class reads in config params from a standard sipX config file
# and creates a map of param-values that is publicly available via the "[]"
# operator.  This class is generic, it is not specifically for Call Resolver.

require 'logger'

require 'utils/exceptions'


class Configure
  
  # Config file syntax
  COMMENT_CHAR = '#'
  SEPARATOR_CHAR = ':'    # separate param names from values
  
  public
  
  # String constants
  DISABLE = 'DISABLE'
  ENABLE = 'ENABLE'
  
  # Initialize inputs:
  #   config_file - relative or absolute path to config file (String)
  #   log - Logger instance to log to
  def initialize(config_file = nil, log = nil)
    @config_file = config_file
    
    if log
      @log = log
    else
      @log = Logger.new(STDOUT)
      @log.level = Logger::ERROR      
    end
    
    # create a Hash to hold the config param name to value map
    @map = {}
    
    # parse the config file and fill in the map
    parse_config(@config_file) if config_file
  end
  
  # Return the value for the named parameter, or nil if no such parameter exists
  def [](param_name)
    return @map[param_name]
  end
  
  
  def enabled?(param_name, default = nil)
    value = @map.fetch(param_name, default)
    raise ConfigException, "No value for #{param_name}" unless value
    case value.upcase
      when ENABLE: true
      when DISABLE: false
      else raise ConfigException, %Q/Unrecognized value "#{value}" for "#{param_name}"./
    end
  end
  
  attr_reader :config_file
  
  private
  
  # Set param values, mainly for testing.
  def []=(param_name, param_value)
    @map[param_name] = param_value
  end
  
  # Parse the config file and construct a hash mapping param names to values.
  def parse_config(config_file)
    begin
      File.open(config_file, 'r') do |file|
        @log.debug("Parsing config file #{config_file}")  
        
        line_num = 0
        file.each_line do |line|
          line_num += 1
          name, value = parse_line(line, line_num)
          next unless name
          @log.debug("Name = #{name}, value=#{value}, line number #{line_num}")
          @map[name] = value unless value.empty?
        end
      end
    rescue SystemCallError
      @log.error("Error opening \"#{config_file}\": #{$!}")
      raise
    end
  end
  
  def parse_line(line, line_num = 0)
    # discard any trailing comments
    line = line.split(COMMENT_CHAR, 2)[0].strip
    return if line.empty?
    
    # split the line into name and value
    name, value = line.split(SEPARATOR_CHAR, 2)
    
    if !value
      msg = "Missing \"#{SEPARATOR_CHAR}\" on line #{line_num}: #{line}"
      @log.error(msg)
      raise(ConfigException, msg)
    end
    
    return name.strip, value.strip
  end  
  
end
