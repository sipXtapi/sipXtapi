// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <net/HttpMessage.h>
#include <net/SdpBody.h>
#include <upnp/UPnpDiscovery.h>
#include <upnp/UPnpService.h>
#include <upnp/UPnpControl.h>
#include <upnp/UPnpAgent.h>

/**
* Unit test for SdpBody
*/

class UpnpTest : public CppUnit::TestCase, public IUPnpNotifier
{
    CPPUNIT_TEST_SUITE(UpnpTest);
    CPPUNIT_TEST(testDiscovery);
    CPPUNIT_TEST_SUITE_END();
public:

    // UPnp notification callback
    void notifyUpnpStatus(bool bSuccess, UPnpBindingTask* const pNotifyingTask)
    {
        if (bSuccess)
            UPnpAgent::getInstance()->setAvailable(true);
        else 
            UPnpAgent::getInstance()->setAvailable(false);
            
        printf("UPnP Status:  %d\n", bSuccess);
        // we are done with the background upnp task
        delete pNotifyingTask;
    }
    void testDiscovery()
    {
        /*
        UPnpDiscovery discovery;
        UtlString location = discovery.discoverRootLocation();
        
        CPPUNIT_ASSERT(location.contains("http://") == UtlBoolean(true));

        location = discovery.discoverGatewayLocation();
        CPPUNIT_ASSERT(location.contains("http://") == UtlBoolean(true));

        location = discovery.discoverWANIPConnectionLocation();
        CPPUNIT_ASSERT(location.contains("http://") == UtlBoolean(true));

        UPnpService service(Url(location.data(), true), UPnpService::WANIPConnection);

        int internalPort = 58764;
        int externalPort = 59764;

        // get our internal ip
        UtlString hostIp;
        OsSocket::getHostIp(&hostIp);

        // create control url based on the WANIPConnection location
        Url controlUrl(service.getControlUrl(), true);

        // create a UPnpControl for the controlURL
        UPnpControl control(controlUrl);

        bool bRet;

        // delete, just in case
        bRet = control.deletePortMapping(externalPort);

        // get our external IP address
        UtlString externalIp;
        bRet = control.getExternalIp(externalIp);
        CPPUNIT_ASSERT(true == bRet);
        CPPUNIT_ASSERT(externalIp.length() > 0);

        // get a port mapping for our external port
        // (should not be able to get one, because deletePortMapping
        // was called above)
        PortMapping portMapping;
        bRet = control.getPortMappingEntry(externalPort, &portMapping);
        CPPUNIT_ASSERT(false == bRet);

        // add our port mapping
        bRet = control.addPortMapping(externalPort, internalPort, hostIp.data(), 0);
        CPPUNIT_ASSERT(true == bRet);

        // get the portMapping that we just added
        bRet = control.getPortMappingEntry(externalPort, &portMapping);
        CPPUNIT_ASSERT(true == bRet);

        // check the contents of portMapping
        CPPUNIT_ASSERT(portMapping.getExternalPort() == externalPort);
        CPPUNIT_ASSERT(portMapping.getInternalPort() == internalPort);
        CPPUNIT_ASSERT(portMapping.getClientIp().compareTo(& UtlString(hostIp.data())) == 0);

        // delete it to be nice
        bRet = control.deletePortMapping(externalPort);
        CPPUNIT_ASSERT(true == bRet);

        bRet = control.deletePortMapping(externalPort);
        CPPUNIT_ASSERT(bRet == false);
        // test the get last error stuff
        CPPUNIT_ASSERT(control.getLastErrorCode() >= 400);
        CPPUNIT_ASSERT(control.getLastErrorText().length() > 0);
        CPPUNIT_ASSERT(control.getLastErrorDetail().length() > 0);
*/
        // test the UPnpAgent
        int externalPort = -1;
        int internalPort = 5061;
        UtlString hostIp;
        OsSocket::getHostIp(&hostIp);
        
        UPnpAgent::getInstance()->setEnabled(true);
        
        if (!UPnpAgent::getInstance()->isAvailable())
        {
            // if it failed last time, with the same network configuration
            // try in the background
                        
            UPnpBindingTask* pBindingTask = new UPnpBindingTask(hostIp.data(), internalPort, this);
            pBindingTask->start();
            OsTask::delay(30000);      
        }
        externalPort = UPnpAgent::getInstance()->bindToAvailablePort(hostIp.data(), internalPort);
        CPPUNIT_ASSERT(externalPort > 1023 && externalPort < 65536);

        int lastUsedExternalPort = externalPort;
        externalPort = -1;
        externalPort = UPnpAgent::getInstance()->bindToAvailablePort(hostIp.data(), internalPort);
        CPPUNIT_ASSERT(externalPort == lastUsedExternalPort);

        UPnpAgent::getInstance()->removeBinding(hostIp.data(), internalPort);

    } 
};

CPPUNIT_TEST_SUITE_REGISTRATION(UpnpTest);
