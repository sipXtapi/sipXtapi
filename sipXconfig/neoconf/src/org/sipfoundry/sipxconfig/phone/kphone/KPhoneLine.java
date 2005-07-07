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

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.AbstractLine;
import org.sipfoundry.sipxconfig.setting.Setting;

public class KPhoneLine extends AbstractLine {
    
    public static final String FACTORY_ID = "kphoneLine";
        
    private static final String REGISTRATION = "Registration";
    
    public KPhoneLine() {
        setModelFilename(KPhone.FACTORY_ID + "/line.xml");
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