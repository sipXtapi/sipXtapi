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

import org.sipfoundry.sipxconfig.phone.AbstractLine;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

public class KPhoneLine extends AbstractLine {
    
    public static final String FACTORY_ID = "kphoneLine";
        
    public Setting getSettingModel() {
        String systemDirectory = getPhoneContext().getSystemDirectory();
        File modelDefsFile = new File(systemDirectory + '/' + KPhone.FACTORY_ID + "/line.xml");
        Setting model = new XmlModelBuilder(systemDirectory).buildModel(modelDefsFile).copy();
        
        return model;
    }
    
    public Object getAdapter(Class interfac) {
        Object impl;
        if (interfac == LineSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(interfac);
            adapter.setSetting(getSettings());
            adapter.addMapping(LineSettings.USER_ID, "Registration/UserName");            
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "Registration/SipServer");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        
        return impl;        
    }
    
    public void setDefaults() {
        super.setDefaults();
        
        getSettings().getSetting("Registration/SipUri").setValue(getUri());
    }
}
