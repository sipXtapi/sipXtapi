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
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Organization;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.PhoneMetaData;
import org.sipfoundry.sipxconfig.phone.LineMetaData;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.ValueStorage;


public class PolycomTestHelper {
    
    PolycomSupport polycom;
    
    PolycomLine[] line;
    
    PolycomPhone[] phone;
    
    User[] user;
    
    ValueStorage[] valueStorage;
    
    public static PolycomTestHelper plainEndpointSeed() throws Exception {
        
        MockControl coreControl = MockControl.createNiceControl(CoreContext.class);
        CoreContext core = (CoreContext) coreControl.getMock();
        Organization rootOrg = new Organization();
        rootOrg.setDnsDomain("sipfoundry.org");
        coreControl.expectAndReturn(core.loadRootOrganization(), rootOrg, MockControl.ZERO_OR_MORE);
        coreControl.replay();
        
        PolycomTestHelper helper = new PolycomTestHelper();
        helper.polycom = new PolycomSupport();
        helper.polycom.setCoreContext(core);
        helper.polycom.setSystemDirectory(TestHelper.getSysDirProperties().getProperty("sysdir.etc"));
        
        PhoneMetaData meta = new PhoneMetaData(PolycomModel.MODEL_600.getModelId());
        meta.setSerialNumber("0004f200e06b");
        helper.phone = new PolycomPhone[] { new PolycomPhone(helper.polycom, meta) };
        
        helper.user = new User[] { new User() };
        helper.user[0].setDisplayId("juser");
        helper.user[0].setFirstName("Joe");
        helper.user[0].setLastName("User");
        
        helper.line = new PolycomLine[] { new PolycomLine(helper.phone[0]) };
        
        LineMetaData lineMeta = helper.line[0].getLineMetaData();
        lineMeta.setUser(helper.user[0]);
        helper.phone[0].addLine(helper.line[0]);
        
        helper.phone[0].setTftpRoot(TestHelper.getTestDirectory());
        helper.phone[0].setVelocityEngine(TestHelper.getVelocityEngine());
        
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
