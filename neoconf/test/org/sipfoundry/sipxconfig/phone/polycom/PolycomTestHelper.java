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
package org.sipfoundry.sipxconfig.phone.polycom;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.LineData;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneData;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.ValueStorage;


public class PolycomTestHelper {
    
    PhoneContext phoneContext;
    
    PolycomLine[] line;
    
    PolycomPhone[] phone;
    
    User[] user;
    
    ValueStorage[] valueStorage;
    
    public static PolycomTestHelper plainEndpointSeed() throws Exception {
        
        PolycomTestHelper helper = new PolycomTestHelper();

        MockControl phoneContextControl = MockControl.createNiceControl(PhoneContext.class);
        helper.phoneContext = (PhoneContext) phoneContextControl.getMock();
        phoneContextControl.expectAndReturn(helper.phoneContext.getDnsDomain(), "sipfoundry.org", MockControl.ZERO_OR_MORE);
        String sysdir = TestHelper.getSysDirProperties().getProperty("sysdir.etc");
        phoneContextControl.expectAndReturn(helper.phoneContext.getSystemDirectory(), sysdir, MockControl.ZERO_OR_MORE);
        
        PhoneData meta = new PhoneData(PolycomModel.MODEL_600.getModelId());
        meta.setSerialNumber("0004f200e06b");
        helper.phone = new PolycomPhone[] { new PolycomPhone() };
        helper.phone[0].setPhoneContext(helper.phoneContext);
        helper.phone[0].setPhoneData(meta);
        helper.phone[0].setOutboundProxyAddress("sipx.sipfoundry.org");
        
        helper.user = new User[] { new User() };
        helper.user[0].setDisplayId("juser");
        helper.user[0].setFirstName("Joe");
        helper.user[0].setLastName("User");
        phoneContextControl.expectAndReturn(helper.phoneContext.getClearTextPassword(helper.user[0]), "1234", MockControl.ZERO_OR_MORE);
        
        helper.line = new PolycomLine[] { new PolycomLine() };
        helper.line[0].setPhone(helper.phone[0]);
        helper.line[0].setLineData(new LineData());

        // for alls to phone.newLine
        PolycomLine newLine = new PolycomLine();
        newLine.setPhone(helper.phone[0]);
        newLine.setLineData(new LineData());
        phoneContextControl.expectAndReturn(helper.phoneContext.newLine(PolycomLine.FACTORY_ID), newLine);
        
        LineData lineMeta = helper.line[0].getLineData();
        lineMeta.setUser(helper.user[0]);
        helper.phone[0].addLine(helper.line[0]);
        
        helper.phone[0].setTftpRoot(TestHelper.getTestDirectory());
        helper.phone[0].setVelocityEngine(TestHelper.getVelocityEngine());
        
        phoneContextControl.replay();
        
        return helper;
    }
    
    public void plainSettingsSeed() throws Exception {
        Setting endpointSettings = phone[0].getSettings();
        endpointSettings.getSetting("call").getSetting("rejectBusyOnDnd").setValue("0");
        endpointSettings.getSetting("voIpProt.SIP").getSetting("local").getSetting("port").setValue("5061");

        Setting lineSettings = line[0].getSettings();
        lineSettings.getSetting("call").getSetting("serverMissedCall").getSetting("enabled").setValue("1");
    }
}
