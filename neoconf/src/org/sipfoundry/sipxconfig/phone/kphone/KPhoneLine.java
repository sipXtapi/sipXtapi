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
package org.sipfoundry.sipxconfig.phone.kphone;

import java.io.File;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.AbstractLine;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

public class KPhoneLine extends AbstractLine {
    
    public static final String FACTORY_ID = "kphoneLine";
        
    private static final String REGISTRATION = "Registration";
    
    public Setting getSettingModel() {
        String systemDirectory = getPhoneContext().getSystemDirectory();
        File modelDefsFile = new File(systemDirectory + '/' + KPhone.FACTORY_ID + "/line.xml");
        Setting model = new XmlModelBuilder(systemDirectory).buildModel(modelDefsFile).copy();
        
        return model;
    }

    public void setDefaults(Setting setting) {
        User user = getLineData().getUser();
        Setting reg = setting.getSetting(REGISTRATION);
        if (user != null) {
            reg.getSetting("UserName").setValue(user.getDisplayId());            
        }
        reg.getSetting("SipUri").setValue(getUri());
    }
}
