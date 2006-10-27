#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'logger'



class SipxFormatter
  
  def call(severity, time, progname, msg)
    puts severity
    puts severity.inspect
    sipx_severity = SipxLogger::LOG_LEVEL_LOGGER_TO_SIPX[severity]
    puts sipx_severity
    "#{sipx_severity}: #{msg}\n"
  end
  
end


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
  
  # Initializer takes any args and passes them through to Logger
  def initialize(*args)
    super
  end
  
  # Given a Logger log level, return the matching sipX log level, or
  # nil if the name is not recognized.
  
  
end
