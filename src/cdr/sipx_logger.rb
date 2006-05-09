#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'logger'


# Subclass the Logger to add richer logging.  The longer-term solution is likely
# to be switching to log4r.
#
# Rails somehow modifies the default logging format so that all you get is the
# message text.  This class assumes that the modification is in effect.
class SipxLogger < Logger
  
  # Sipx log levels
  SIPX_DEBUG   = 'DEBUG' 
  SIPX_INFO    = 'INFO'
  SIPX_NOTICE  = 'NOTICE'
  SIPX_WARNING = 'WARNING'
  SIPX_ERR     = 'ERR'
  SIPX_CRIT    = 'CRIT'
  SIPX_ALERT   = 'ALERT'
  SIPX_EMERG   = 'EMERG'
  
  # Map the names of sipX log levels (DEBUG, INFO, NOTICE, WARNING, ERR, CRIT,
  # ALERT, EMERG) to equivalent Logger log levels.
  LOG_LEVEL_SIPX_TO_LOGGER = {
    SIPX_DEBUG   => DEBUG, 
    SIPX_INFO    => INFO, 
    SIPX_NOTICE  => INFO, 
    SIPX_WARNING => WARN,
    SIPX_ERR     => ERROR, 
    SIPX_CRIT    => FATAL,
    SIPX_ALERT   => FATAL,
    SIPX_EMERG   => FATAL
  }
  
  # Map from Logger log levels to sipX log levels.  Because there are fewer
  # Logger log levels, this mapping loses information.
  LOG_LEVEL_LOGGER_TO_SIPX = {
    DEBUG => SIPX_DEBUG, 
    INFO  => SIPX_INFO, 
    WARN  => SIPX_WARNING,
    ERROR => SIPX_ERR, 
    FATAL => SIPX_CRIT
  }

public

  # Initializer takes any args and passes them through to Logger
  def initialize(*args)
    super
  end

  # Override Logger::add to print the severity at the front of the message
  def add(severity, message = nil, progname = nil, &block)
    # Compute the log message.  This code is unfortunately copied directly from
    # the Logger implementation -- I don't know a better way to do this.
    severity ||= UNKNOWN
    if @logdev.nil? or severity < @level
      return true             # early returns are bad style, but let it go
    end
    progname ||= @progname
    if message.nil?
      if block_given?
        message = yield
      else
        message = progname
        progname = @progname
      end
    end
    
    # Put the sipX log level in front of the message
    message = "#{log_level_logger_to_sipx(severity)}: " + message
    
    # Let the base class do the rest. Since we have already evaluated the block
    # above (if present), we don't need it here.
    super(severity, message, progname)
  end

private

  # Given a Logger log level, return the matching sipX log level, or
  # nil if the name is not recognized.
  def log_level_logger_to_sipx(name)
    LOG_LEVEL_LOGGER_TO_SIPX[name]
  end
  
end
