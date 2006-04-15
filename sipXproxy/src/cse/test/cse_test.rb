#
# Copyright (C) 2006 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
# 
# Copyright (C) 2006 Pingtel Corp.
# Licensed to SIPfoundry under a Contributor Agreement.
#
##############################################################################

# system requirements
require 'environment'
require 'sipsupport'
require 'test/unit'

class CallTestSuccessful < Test::Unit::TestCase
  def setup
    setEnvironment
    assert(startServices, \
           'Failure starting up services or retrieving process Ids')
  end

  def teardown
    stopServices
    assert(checkForZombieProxies, \
           'Failure shutting down services')
  end

  def test_SubsequentSuccessfulCalls
    callId = 0;
    callIdPrefix = "callId-"

    proxy = SipConnection.new($hostName, $proxyPort)
    authproxy = SipConnection.new($hostName, $authproxyPort)

    # Loop through all the test URIs we generated and let everybody
    # call everyone else - but don't let them call themselves
    $testSipURIs.each do |toUri|
      $testSipURIs.each do |fromUri|
        if toUri != fromUri
          callIdString = "#{callIdPrefix}" + callId.to_s
          puts "Call from #{fromUri} to #{toUri} (#{callIdString})" \
                if $verbose
          call = SipCall.new(proxy, authproxy, \
                             callIdString, toUri, fromUri)
          call.makeSuccessful(1)
          callId += 1
        end
      end
    end
  end
end

