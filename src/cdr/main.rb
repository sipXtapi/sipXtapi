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
require File.join(File.dirname(__FILE__), 'call_resolver')

require 'call_direction_plugin'


# Parse command-line options
#   start: date/time from which to start analyzing call events
#   end:   date/time at which to end the analysis (defaults to 1 day later)
#   redo:  if true then calculate all CDRs, even those already done before
#          (defaults to false)
# :TODO: Print out a helpful message if the caller enters no options or screws up
opts = GetoptLong.new(
  [ "--start", "-s", GetoptLong::REQUIRED_ARGUMENT ],
  [ "--end",   "-e", GetoptLong::OPTIONAL_ARGUMENT ],
  [ "--redo",  "-r", GetoptLong::NO_ARGUMENT ]
)

# Init options
start_time = end_time = nil
redo_flag = false

# Extract option values
# Convert start and end strings to date/time values.
opts.each do |opt, arg|
  case opt

  when "--start"
    start_time = Time.parse(arg)

  when "--end"
    end_time = Time.parse(arg)

  when "--redo"
    redo_flag = true

  end
end 

# Create a Call Resolver
resolver = CallResolver.new

# Add the Call Direction Plugin as an observer so that it can compute call direction
# *** temporarily disabled ***
#resolver.add_observer(CallDirectionPlugin.new(resolver))

# Resolve calls that occurred during the specified time window
resolver.resolve(start_time, end_time, redo_flag)
