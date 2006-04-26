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

import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;

public class KPhone extends Phone {
    
    public static final PhoneModel MODEL_KPHONE = new PhoneModel("kphone", "KPhone");
    
    public KPhone() {
        super(MODEL_KPHONE);
        setPhoneTemplate("kphone/kphonerc.vm");
    }
    
    public String getPhoneFilename() {
        return getWebDirectory() + "/" + getSerialNumber() + ".kphonerc";
    }    
    
    public Object getLineAdapter(Line line, Class interfac) {
        Object impl;
        if (interfac == LineSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(interfac);
            adapter.setSetting(line.getSettings());
            adapter.addMapping(LineSettings.USER_ID, "Registration/UserName");            
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "Registration/SipServer");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        
        return impl;        
    }
    
    public void defaultLineSettings(Line line) {
        super.defaultLineSettings(line);
        
        line.getSettings().getSetting("Registration/SipUri").setValue(line.getUri());
    }
}
