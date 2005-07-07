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

import java.io.File;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.LineData;
import org.sipfoundry.sipxconfig.phone.PhoneData;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;


public class PolycomTestHelper {
    
    PolycomSupport polycom;
    
    PolycomLine[] line;
    
    PolycomPhone[] phone;
    
    User[] user;
    
    ValueStorage[] valueStorage;
    
    public static PolycomTestHelper plainEndpointSeed() throws Exception {
        
        PolycomTestHelper helper = new PolycomTestHelper();

        MockControl polycomControl = MockControl.createNiceControl(PolycomSupport.class);
        helper.polycom = (PolycomSupport) polycomControl.getMock();
        polycomControl.expectAndReturn(helper.polycom.getDnsDomain(), "sipfoundry.org");
        String sysdir = TestHelper.getSysDirProperties().getProperty("sysdir.etc");
        polycomControl.expectAndReturn(helper.polycom.getSystemDirectory(), sysdir);

        File lineFile = new File(sysdir + "/polycom/line.xml");
        Setting lineModel = new XmlModelBuilder().buildModel(lineFile);
        polycomControl.expectAndReturn(helper.polycom.getLineSettingModel(), lineModel, MockControl.ZERO_OR_MORE);

        File phoneFile = new File(sysdir + "/polycom/phone.xml");
        Setting endpointModel = new XmlModelBuilder().buildModel(phoneFile);
        polycomControl.expectAndReturn(helper.polycom.getEndpointSettingModel(), endpointModel, MockControl.ZERO_OR_MORE);
        
        PhoneData meta = new PhoneData(PolycomModel.MODEL_600.getModelId());
        meta.setSerialNumber("0004f200e06b");
        helper.phone = new PolycomPhone[] { new PolycomPhone(helper.polycom, meta) };
        
        helper.user = new User[] { new User() };
        helper.user[0].setDisplayId("juser");
        helper.user[0].setFirstName("Joe");
        helper.user[0].setLastName("User");
        polycomControl.expectAndReturn(helper.polycom.getClearTextPassword(helper.user[0]), "1234", MockControl.ZERO_OR_MORE);
        
        helper.line = new PolycomLine[] { new PolycomLine(helper.phone[0], new LineData()) };
        
        LineData lineMeta = helper.line[0].getLineData();
        lineMeta.setUser(helper.user[0]);
        helper.phone[0].addLine(helper.line[0]);
        
        helper.phone[0].setTftpRoot(TestHelper.getTestDirectory());
        helper.phone[0].setVelocityEngine(TestHelper.getVelocityEngine());
        
        polycomControl.replay();
        
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
