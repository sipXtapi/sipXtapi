/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package com.pingtel.pds.pgs.phone;

import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.rmi.RemoteException;

import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.Difference;
import org.custommonkey.xmlunit.DifferenceListener;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.easymock.MockControl;
import org.w3c.dom.Node;

import com.pingtel.pds.pgs.organization.OrganizationBusiness;
import com.pingtel.pds.pgs.profile.RefPropertyBusiness;

/**
 * DeviceHelperTest
 */
public class DeviceHelperTest extends XMLTestCase {

    public DeviceHelperTest() {
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testCalculateDeviceLineUrl() throws RemoteException {
        
        MockControl deviceControl = MockControl.createNiceControl(DeviceBusiness.class);
        DeviceBusiness device = (DeviceBusiness) deviceControl.getMock();
        deviceControl.expectAndReturn(device.getShortName(),"short_name");
        deviceControl.expectAndReturn(device.getSerialNumber(),"00D01E001414");
        deviceControl.replay();

        MockControl orgControl = MockControl.createNiceControl(OrganizationBusiness.class);
        OrganizationBusiness org = (OrganizationBusiness) orgControl.getMock();
        orgControl.expectAndReturn(org.getDNSDomain(),"mycomp.com");
        orgControl.replay();
        
        DeviceHelper helper = new DeviceHelper(device);
        String url = helper.calculateDeviceLineUrl(org);
        assertEquals("short_name<sip:00D01E001414@mycomp.com>",url);
        
        orgControl.verify();
    }

    public void testCreateInitialLine() throws Exception {
        MockControl orgControl = MockControl.createNiceControl(OrganizationBusiness.class);
        OrganizationBusiness org = (OrganizationBusiness) orgControl.getMock();
        orgControl.expectAndReturn(org.getDNSDomain(),"mycomp.com",2);
        orgControl.expectAndReturn(org.getAuthenticationRealm(),"auth.mycomp.com",2);
        orgControl.replay();

        MockControl deviceControl = MockControl.createControl(DeviceBusiness.class);
        DeviceBusiness device = (DeviceBusiness) deviceControl.getMock();
        deviceControl.expectAndReturn(device.getSerialNumber(),"00D01E001414",3);
        deviceControl.expectAndReturn(device.getShortName(),"short_name");
        deviceControl.replay();
        
        MockControl rpControl = MockControl.createNiceControl(RefPropertyBusiness.class);
        RefPropertyBusiness rp = (RefPropertyBusiness) rpControl.getMock();
        rpControl.expectAndReturn(rp.getID(),new Integer(5));
        rpControl.replay();
        
        DeviceHelper helper = new DeviceHelper(device);
        String generatedXml = helper.createInitialDeviceLine(org,rp);
        
        Reader controlXml = new InputStreamReader(getClass().getResourceAsStream("line_profile.test.xml"));
        
        Diff diff = new Diff(controlXml,new StringReader(generatedXml));
        diff.overrideDifferenceListener(new SkipPasstoken());
        assertXMLEqual(diff,true);
                
        deviceControl.verify();
        orgControl.verify();
        rpControl.verify();
    }
    
    
    public static class SkipPasstoken implements DifferenceListener
    {
        public int differenceFound(Difference difference) {
            
            Node node = difference.getControlNodeDetail().getNode();
            String parentName = node.getParentNode().getNodeName();
            if ( parentName.equalsIgnoreCase("PASSTOKEN") )
            {
                return RETURN_IGNORE_DIFFERENCE_NODES_IDENTICAL;
            }
            return RETURN_ACCEPT_DIFFERENCE;
        }

        public void skippedComparison(Node control, Node test) {
            // intentionally empty
        }
    }
}
