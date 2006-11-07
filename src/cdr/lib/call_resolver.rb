#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require 'logger'

require 'utils/call_resolver_configure'
require 'call_direction/call_direction_plugin'
require 'db/cse_reader'
require 'db/cdr_writer'
require 'state'


# The CallResolver analyzes call state events (CSEs) and computes call detail 
# records (CDRs).  It loads CSEs from a database and writes CDRs back into the
# same database.
# :TODO: log the number of calls analyzed and how many succeeded vs. dups or
#        failures, also termination status
class CallResolver
  attr_reader :log, :config
  
  # How many seconds are there in a day
  SECONDS_IN_A_DAY = 86400
  
  def initialize(config_file = nil)
    @config =  CallResolverConfigure.from_file(config_file)
    @log = config.log
  end
    
  # Run daily processing, including purging and/or call resolution
  def daily_run(purge_flag = false, purge_time = 0)
    run_resolver
    run_purge(purge_time) if purge_flag || config.purge?
  end
  
  def run_resolver
    if !config.daily_run?
      log.error("resolve: the --daily_flag is set, but the daily run is disabled in the configuration");
      return
    end
    
    start_time, end_time = get_daily_start_time
    
    begin
      log.debug{"Daily run - resolve start: #{start_time}, end: #{end_time}"}
      log.info("Resolving calls from #{start_time.to_s} to #{end_time.to_s}")
      resolve(start_time, end_time)
      log.info("resolve: Done at #{end_run}.  Analysis took #{Time.now - start_run} seconds.")
    rescue
      # FIXME: check if we can remove it or at least rethrow exception
      # Log the error and the stack trace.  The log message has to
      # go on a single line.  
      # Embed "\n" for syslogviewer.
      start_line = "\n        from "    # start each backtrace line with this
      log.error("Exiting because of error: \"#{$!}\"" + start_line +
      $!.backtrace.inject{|trace, line| trace + start_line + line})            
    end      
  end
  
  def run_purge()
    # Was a purge age explicitly set?
    if purge_time != 0
      log.info("Purge override")
      start_cse = start_cdr = Time.now() - (SECONDS_IN_A_DAY * purge_time)
    else
      start_cse = config.purge_start_time_cse
      start_cdr = config.purge_start_time_cdr
      log.info("Normal purge")
    end 
    log.info("Start CSE: : #{start_cse}, Start CDRs: #{start_cdr}")
    @readers.each{ |r| r.purge_cse(start_cse) }
    @writer.purge_cdr(start_cdr)
  end
  
  # Resolve CSEs to CDRs
  def resolve(start_time, end_time)
    cse_queue = Queue.new
    urls = @config.cse_database_urls
    
    # readers put events in CSE queue
    @readers = urls.collect do | url |
      CseReader.new( cse_queue, url )
    end
    
    cdr_queue = Queue.new
    
    
    #cdr_queue = start_plugins(cdr_queue)
        
    @writer = CdrWriter.new(cdr_queue, @config.cdr_database_url)
    
    
    # start everything

    writer_thread = Thread.new( @writer ) { | w | w.run }
    
    
    reader_threads = @readers.collect do | reader |
      Thread.new(reader) { |r| r.run(start_time, end_time) }
    end

    Thread.new( cse_queue, cdr_queue ) { | inq, outq | 
      # state copies from CSE queue to CDR queue
      s = State.new( cse_queue, cdr_queue )
      s.run
    }
    
    reader_threads.each{ |thread| thread.join }
    
    # send sentinel event, it will stop plugins and state
    cse_queue.enq(nil)
    
    # wait for writer before exiting
    writer_thread.join
  end
  

  def start_plugins(raw_queue)
    return raw_queue unless CallDirectionPlugin.call_direction?(@config)    
    processed_queue = Queue.new
    
    cdp = CallDirectionPlugin.new(raw_queu, processed_queue)  
    Thread.new(cdp) { | plugin | plugin.run }
    
    return cdr_queue
  end
  
end
