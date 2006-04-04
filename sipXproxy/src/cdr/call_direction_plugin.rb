#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# application requires
require File.join(File.dirname(__FILE__), 'call_resolver')


# CallDirectionPlugin computes call direction for a CDR, a customer-specific
# requirement.  We do this in a plugin to keep the core CallResolver generic.
# :NOW: Read call direction config param and compute call direction only if
# that param is set
class CallDirectionPlugin
  
public
  SIPXCONFIG_DATABASE = 'SIPXCONFIG'
  GATEWAY_TABLE_NAME = 'gateway'
  
  def initialize(resolver)
    @resolver = resolver

    # Gateways are stored in the SIPXCONFIG database. Connect to that database.
    # Reuse an existing open connection if one is open. The hitch is that we
    # can't simply use the SIPXCDR connection inherited from ActiveRecord::Base,
    # we need a SIPXCONFIG connection. Reuse the open connection only if it has
    # a gateway table, which indicates a connection to a SIPXCONFIG database.
    if !Gateway.connected? or
       !Gateway.connection.tables.find {|table| table == GATEWAY_TABLE_NAME}
      Gateway.establish_connection(
        :adapter  => "postgresql",
        :host     => "localhost",
        :username => "postgres",
        :database => SIPXCONFIG_DATABASE)
    end
    
    @gateways = Gateway.find(:all)
    if log.debug?
      if @gateways and (@gateways.length > 0)
        log.debug("Found #{@gateways.length} gateways:")
        @gateways.each do |g|
          log.debug("  name = #{g.name}, address = #{g.address}")
        end
      else
        log.debug("No gateways found")
      end
    end
  end
  
  # When CallResolver tells us that a new CDR has been created, compute and
  # save call direction for that CDR.
  def update(event_type,    # Call Resolver event type
             cdr)           # Cdr object
    
    # The "new CDR" event is the only event type handled by this plugin
    if event_type == CallResolver::EVENT_NEW_CDR
      log.debug("CallDirectionPlugin#update: CDR has call ID = #{cdr.call_id}, " +
                "ID = #{cdr.id}")
      
      #find_call_direction(cdr)
    end
  end

private

  # Find call direction for the CDR and save it
  def find_call_direction(cdr)
    # Check if the from or to contact is the address of a gateway.
  end

  # Share the Call Resolver's Logger
  def log
    @resolver.log
  end
  
end
