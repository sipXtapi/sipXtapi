#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'rubygems'            # Ruby packaging and installation framework
require_gem 'activerecord'    # object-relational mapping layer for Rails
require 'logger'              # writes log messages to a file or stream

module Cdr

=begin rdoc
The CallResolver analyzes call state events (CSEs) and computes call detail 
records (CDRs).  It loads CSEs from a database and writes CDRs back into the
same database.
=end
class CallResolver
  def initialize
    # Set up logging.
    # :TODO: write to $SIPX_PREFIX/var/log/sipxpbx/sipcallresolver.log
    # :TODO: figure out interactions between Ruby log rotation and sipX log rotation
    @@log = Logger.new(STDOUT)

    if $DEBUG then
      @@log.level = Logger::DEBUG
    else
      @@log.level = Logger::INFO
    end
  end

  # Resolve CSEs to CDRs
  def resolve(start_time, end_time, redo_flag)
    @@log.info "resolve: start = #{start_time.to_s}, end = #{end_time.to_s}, redo = #{redo_flag}"
  end

end    # class CallResolver

end    # module Cdr
