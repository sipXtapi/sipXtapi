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

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.User;


public class PolycomTestHelper {
    
    Endpoint[] endpoint;
    
    PolycomPhone[] phone;
    
    User[] user;
    
    public static PolycomTestHelper plainEndpointSeed() throws Exception {
        PolycomTestHelper helper = new PolycomTestHelper();
        helper.phone = new PolycomPhone[] { new PolycomPhone() };
        helper.phone[0].setSystemDirectory(TestHelper.getSysDirProperties().getProperty("sysdir.etc"));
        helper.phone[0].setModelId(Polycom.MODEL_600.getModelId());

        helper.endpoint = new Endpoint[] { new Endpoint() };
        helper.endpoint[0].setSerialNumber("0004f200e06b");
        helper.endpoint[0].setPhoneId(helper.phone[0].getModelId());
        
        helper.user = new User[] { new User() };
        helper.user[0].setDisplayId("Joe User");
        helper.user[0].setFirstName("Joe");
        helper.user[0].setLastName("User");
        
        Line line = new Line();
        line.setUser(helper.user[0]);
        helper.endpoint[0].addLine(line);
        
        PolycomPhoneConfig config = new PolycomPhoneConfig();
        config.setTftpRoot(TestHelper.getTestDirectory());
        config.setVelocityEngine(TestHelper.getVelocityEngine());
        helper.phone[0].setConfig(config);
        
        return helper;
    }

}
