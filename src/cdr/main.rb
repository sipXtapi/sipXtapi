#!/usr/bin/env ruby
#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# system requires
require 'getoptlong'
require 'parsedate'

# application requires
require 'call_resolver'
require 'utils/stunnel_connection'

resolver = CallResolver.new

# Parse command-line options
#   start: date/time from which to start analyzing call events
#   end:   date/time at which to end the analysis (defaults to 1 day later)
#   daily: perform daily processing based on the configuration -- resolve
#          calls and/or purge old data
opts = GetoptLong.new(
                      [ "--start", "-s", GetoptLong::OPTIONAL_ARGUMENT ],
[ "--end",   "-e", GetoptLong::OPTIONAL_ARGUMENT ],
[ "--daily", "-d", GetoptLong::NO_ARGUMENT ],
[ "--help",  "-h", GetoptLong::NO_ARGUMENT ]
)

# Init options
start_time = end_time = nil
purge_flag = false
purge_time = 0
daily_flag = false

# Extract option values
# Convert start and end strings to date/time values.
opts.each do |opt, arg|
  case opt
    
  when "--start"
    start_time = Time.parse(arg)
    
  when "--end"
    end_time = Time.parse(arg)
    
  when "--daily"
    daily_flag = true
    
  else
    usage
  end
end 

stunnel_connection = StunnelConnection.new(resolver.log)

begin
  stunnel_connection.open(resolver.config)
  
  if daily_flag
    resolver.daily_run
  elsif start_time && end_time
    resolver.resolve(start_time, end_time)
  else
    usage    
  end  
  
rescue
  # Backstop exception handler: don't let any exceptions propagate back
  # to the caller.  
  # Embed "\n" for syslogviewer.
  resolver.log.error do
    start_line = "\n  from "    # start each backtrace line with this
    msg = %Q<Exiting because of error: "#{$!}" > + start_line
    $!.backtrace.inject(msg) {|trace, line| trace + start_line + line}
  end
  
ensure
  stunnel_connection.close
end  


def usage
  print <<EOT
usage: sipxcallresolver.sh [--start "time" [--end "time"]] [--daily] [--help]

  --start "time"  Specifies the start time of the window in which calls are to 
                  be resolved. The time format is ISO 8601.
  --end "time"    Specifies the end time of the window in which call are to be
                  resolved. The time format is ISO 8601.
  --daily         Indicates that the call resolver is called in "daily" mode:
                  * Create CDRs for calls in the previous 24 hours
                  * Purge CSE and CDR data older than 35 days
                  You can change the purge age by setting the configuration
                  parameter SIP_CALLRESOLVER_PURGE_AGE_CDR (units are days).
                  In daily mode, the "start" and "end" args are not used.
EOT
  exit 1
end