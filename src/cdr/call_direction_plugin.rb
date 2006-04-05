#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# Application requires.  Assume that the load path has been set up for us.
require 'call_resolver'
require 'call_direction'
require 'exceptions'


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

    # Find the gateways.  For gateways configured with domain names, resolve the
    # names to IP addresses so we have a canonical format for address matching.
    # Catch any exceptions in name resolution and discard such gateways.
    @gateways = Gateway.find(:all)
    begin
      # Compute the IP address purely for side effect here
      @gateways.each {|g| g.ip_address}
    rescue GatewayNameResolutionException
      g = $!.gateway
      log.error("Unable to resolve the domain name \"#{g.address}\" for the " +
                "gateway named \"#{g.name}\".  This gateway will not be used " +
                "when computing call direction.")
      @gateways.delete(g)
    end
    
    if log.debug?
      # For debugging, list the gateways with their addresses.  If the gateway
      # has a domain name as its address, then list both the domain name and
      # the associated IP address, otherwise just list the IP address.
      if @gateways and (@gateways.length > 0)
        log.debug("Found #{@gateways.length} gateways for computing call direction:")
        @gateways.each do |g|
          str = "  name = #{g.name}, "
          address = g.address
          ip_address = g.ip_address
          if address and (address != ip_address)
            str += "domain name = \"#{address}\", "
          else
            str += "domain name is unknown, "
          end
          str += "IP address = #{ip_address}"
          log.debug(str)
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
      find_call_direction(cdr)
    end
  end

private

  # Find call direction for the CDR and save it
  def find_call_direction(cdr)
    # Compute the call direction, based on whether the from or to contact
    # is a gateway address. At most one of them can be a gateway address.
    direction = CallDirection::INTRANETWORK
    if is_gateway_address(cdr.caller.contact)
      direction = CallDirection::INCOMING
    elsif is_gateway_address(cdr.callee.contact)
      direction = CallDirection::OUTGOING
    end
    log.debug("CallDirectionPlugin#update: CDR has call ID = #{cdr.call_id}, " +
              "ID = #{cdr.id}, direction = #{direction}")
      
    # Save the call direction.  Watch out for preexisting rows in the DB.
    call_direction = CallDirection.find(:first,
                                        :conditions => ["id = :id", {:id => cdr.id}])
    call_direction ||= CallDirection.new(:id => cdr.id)
    call_direction.direction = direction
    call_direction.save
  end

  def is_gateway_address(contact)
    contact_ip_addr = contact_ip_addr(contact)
    @gateways.any? {|g| g.ip_address.equals(contact_ip_addr)}
  end

  def contact_ip_addr(contact)
    # Find the colon at the end of "sip:" or "sips:".  Strip the colon and
    # everything before it.
    colon_index = contact.index(':')
    if !colon_index
      raise(BadContactException.new(contact),
            "Bad contact, can't find colon: \"#{contact}\"",
            caller)
    end
    contact = contact[(colon_index + 1)..-1]
    
    # If there is a semicolon indicating contact params, then strip the params
    # :NOW: This should be done in call resolver itself
    semi_index = contact.index(';')
    if semi_index
      contact = contact[0..semi_index]
    end
    
    # If there is an ">" at the end, then remove it
    gt_index = contact.index('>')
    if gt_index
      contact = contact[0..gt_index]
    end
    
    # If there is another colon indicating a port #, then remove the port #
    colon_index = contact.index(':')
    if colon_index
      contact = contact[0..colon_index]
    end
  end

  # Share the Call Resolver's Logger
  def log
    @resolver.log
  end
  
end
