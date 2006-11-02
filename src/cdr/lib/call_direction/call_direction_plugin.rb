#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################


require 'utils/configure'
require 'utils/exceptions'
require 'call_direction/gateway'

# CallDirectionPlugin computes call direction for a CDR.
class CallDirectionPlugin
  attr_reader :log
  
  public
  
  # Configuration parameters
  CALL_DIRECTION = 'SIP_CALLRESOLVER_CALL_DIRECTION'
  CALL_DIRECTION_DEFAULT = Configure::DISABLE
  
  # Call direction: single-char codes stored in the call_direction column.
  INCOMING = 'I'        # calls that come in from a PSTN gateway
  OUTGOING = 'O'        # calls that go out to a PSTN gateway
  INTRANETWORK = 'A'    # calls that are pure SIP and don't go through a gateway
  
  # cdrin - input queue for CDRs
  # cdrout - output queue for CDRs
  def initialize(cdrin, cdrout, gateway_addresses, log)
    @cdrin = cdrin
    @cdrout = cdrout
    @log = log
    
    @gateway_addresses = gateway_addresses
  end
  
  # process CDR queue setting call direction for each CDR
  def run
    while cdr = @cdrin.deq
      set_call_direction(cdr)
      @cdrout.enq(cdr)
    end
  end
  
  # Compute and set call direction for the CDR.
  def set_call_direction(cdr)
    # Compute the call direction, based on whether the from or to contact
    # is a gateway address. At most one of them can be a gateway address.
    call_direction = case 
    when gateway_address?(cdr.caller_contact): INCOMING
    when gateway_address?(cdr.callee_contact): OUTGOING
    else INTRANETWORK
    end
    log.debug("CallDirectionPlugin#update: CDR has call ID = #{cdr.call_id}, " +
              "caller_contact = #{cdr.caller_contact}, " +
              "callee_contact = #{cdr.callee_contact}, " +
              "call_direction = #{call_direction}")
    
    # Update the CDR's call_direction
    cdr.call_direction = call_direction
  end

  private
    
  def gateway_address?(contact)
    return unless contact
    contact_addr = Utils.contact_host(contact)
    @gateway_addresses.any? {|g| g == contact_addr}
  end
  
  class << self    
    # Return true if call direction is enabled, false otherwise, based on the config.
    def call_direction?(config)
      config.enabled?(CALL_DIRECTION, CALL_DIRECTION_DEFAULT)
    end
    
    # Find the gateways.  For gateways configured with domain names, resolve the
    # names to IP addresses so we have a canonical format for address matching.
    # Return array of resolved IP addresses.
    # FIXME: we should not be connecting to SIPXCONFIG database, it's better to use SOAP or somehow find out from proxy
    def load_and_resolve_gateways
      # Build a gateway IP list
      Gateway.find_all.inject([]) do | addresses, gateway |
        ip_addresses = gateway.ip_addresses
        addresses.concat(ip_addresses) if ip_addresses
      end
    end
  end
    
end
