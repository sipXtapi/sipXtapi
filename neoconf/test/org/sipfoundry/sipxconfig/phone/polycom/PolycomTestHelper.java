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
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.ValueStorage;


public class PolycomTestHelper {
    
    Endpoint[] endpoint;
    
    Line[] line;
    
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
        helper.phone = new PolycomPhone[] { new PolycomPhone() };
        helper.phone[0].setCoreContext(core);
        helper.phone[0].setSystemDirectory(TestHelper.getSysDirProperties().getProperty("sysdir.etc"));
        helper.phone[0].setModelId(Polycom.MODEL_600.getModelId());

        helper.endpoint = new Endpoint[] { new Endpoint() };
        helper.endpoint[0].setSerialNumber("0004f200e06b");
        helper.endpoint[0].setPhoneId(helper.phone[0].getModelId());
        
        helper.user = new User[] { new User() };
        helper.user[0].setDisplayId("juser");
        helper.user[0].setFirstName("Joe");
        helper.user[0].setLastName("User");
        
        helper.line = new Line[] {new Line()};
        helper.line[0].setUser(helper.user[0]);
        helper.endpoint[0].addLine(helper.line[0]);
        
        helper.phone[0].setTftpRoot(TestHelper.getTestDirectory());
        helper.phone[0].setVelocityEngine(TestHelper.getVelocityEngine());
        
        return helper;
    }
    
    public void plainSettingsSeed() throws Exception {
        Setting endpointSettings = endpoint[0].getSettings(phone[0]);
        endpointSettings.getSetting("call").getSetting("rejectBusyOnDnd").setValue("0");
        endpointSettings.getSetting("voIpProt.SIP").getSetting("local").getSetting("port").setValue("5061");

        Setting lineSettings = line[0].getSettings(phone[0]);
        lineSettings.getSetting("call").getSetting("serverMissedCall").getSetting("enabled").setValue("1");
    }
}
