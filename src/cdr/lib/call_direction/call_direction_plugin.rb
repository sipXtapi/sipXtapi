#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

require "dbi"


# Application requires.  Assume that the load path has been set up for us.
require 'call_resolver'
require 'configure'
require 'exceptions'
require 'app/models/gateway'


# CallDirectionPlugin computes call direction for a CDR, a customer-specific
# requirement.  We do this in a plugin to keep the core CallResolver generic.
class CallDirectionPlugin
  
public

  # Configuration parameters
  CALL_DIRECTION = 'SIP_CALLRESOLVER_CALL_DIRECTION'
  CALL_DIRECTION_DEFAULT = Configure::DISABLE
  
  # Call direction: single-char codes stored in the call_direction column.
  INCOMING = 'I'        # calls that come in from a PSTN gateway
  OUTGOING = 'O'        # calls that go out to a PSTN gateway
  INTRANETWORK = 'A'    # calls that are pure SIP and don't go through a gateway

  
  def initialize(resolver)
    @resolver = resolver

    # FIXME: we should not be connecting to SIPXCONFIG datavase, it's better to use SOAP or somehow find out from proxy
    load_and_resolve_gateways    
  end
  
  # When CallResolver tells us that a new CDR has been created, then compute
  # call direction for that CDR.
  def update(cdr)
    set_call_direction(cdr)
  end
  
  # Return true if call direction is enabled, false otherwise, based on the config.
  # :TODO: Factor out the code for ENABLE/DISABLE config params into a shared method.
  def CallDirectionPlugin.call_direction?(config)
    # Look up the config param
    call_direction = config[CALL_DIRECTION]
    
    # Apply the default if the param was not specified
    call_direction ||= CALL_DIRECTION_DEFAULT

    # Convert to a boolean
    if call_direction.casecmp(Configure::ENABLE) == 0
      call_direction = true
    elsif call_direction.casecmp(Configure::DISABLE) == 0
      call_direction = false
    else
      raise(ConfigException, "Unrecognized value \"#{call_direction}\" for " +
            "#{CALL_DIRECTION}.  Must be ENABLE or DISABLE.")
    end
    
    call_direction
  end

private

  # For use by test code
  attr_writer :gateway_addresses

  # Compute and set call direction for the CDR.
  def set_call_direction(cdr)
    # Compute the call direction, based on whether the from or to contact
    # is a gateway address. At most one of them can be a gateway address.
    call_direction = INTRANETWORK
    if cdr.caller_contact and is_gateway_address(cdr.caller_contact)
      call_direction = INCOMING
    elsif cdr.callee_contact and is_gateway_address(cdr.callee_contact)
      call_direction = OUTGOING
    end
    log.debug("CallDirectionPlugin#update: CDR has call ID = #{cdr.call_id}, " +
              "caller_contact = #{cdr.caller_contact}, " +
              "callee_contact = #{cdr.callee_contact}, " +
              "call_direction = #{call_direction}")
      
    # Update the CDR's call_direction
    cdr.call_direction = call_direction
  end

  def is_gateway_address(contact)
    contact_addr = Utils.contact_host(contact)
    @gateway_addresses.any? {|g| g == contact_addr}
  end
  
  def load_and_resolve_gateways
    # Find the gateways.  For gateways configured with domain names, resolve the
    # names to IP addresses so we have a canonical format for address matching.
    # Return array of resolved IP addresses.
    @gateways = Gateway.find_all
    @gateway_addresses = []

    # Build a gateway IP list
    @gateways.each do |gateway|
      ip_addresses = gateway.ip_addresses
      if ip_addresses.length == 0
        log.error("Unable to resolve the domain name \"#{gateway.address}\" for the " +
        "gateway named \"#{gateway.name}\".  This gateway will not be used " +
        "when computing call direction.")
      else
        @gateway_addresses.concat(ip_addresses)
        log.debug do   
          # For debugging, list the gateways with their addresses.  If the gateway
          # has a domain name as its address, then list both the domain name and
          # the associated IP address, otherwise just list the IP address.            
          str = "  name = #{g.name}, "
          address = g.address
          ip_address = ip_addresses
          if address and (address != ip_addresses.to_s)
            str += "domain name = \"#{address}\", "
          else
            str += "domain name is unknown, "
          end
          addr_str = ''
          ip_addresses.each {|a| addr_str += "#{a} "}
          str += "IP address(es) = #{addr_str}"
          str     
        end
      end
    end
    log.debug("Found #{@gateway_addresses.length} gateways for computing call direction.")    
  end

  # Use the Call Resolver's Logger
  def log
    @resolver.log
  end
  
end
