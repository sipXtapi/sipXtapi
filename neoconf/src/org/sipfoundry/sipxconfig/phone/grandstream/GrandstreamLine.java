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
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.io.File;

import org.sipfoundry.sipxconfig.phone.AbstractLine;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Grandstream business functions for line meta setting
 */
public class GrandstreamLine extends AbstractLine {

    public static final String FACTORY_ID = "grandstreamLine";

    public GrandstreamLine() {
        setModelFile("grandstream/line.xml");
    }

    public Setting getSettingModel() {
        String sysPath = getPhoneContext().getSystemDirectory(); 
        File sysDir = new File(sysPath);
        File modelDefsFile = new File(sysDir, getModelFile());
        Setting model = new XmlModelBuilder(sysPath).buildModel(modelDefsFile);
        
        GrandstreamPhone grandstreamPhone = (GrandstreamPhone) getPhone();
        return model.getSetting(grandstreamPhone.getModel().getModelId());
    }
    
    public Object getAdapter(Class interfac) {
        Object impl;
        if (interfac == LineSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(interfac);
            adapter.setSetting(getSettings());
            adapter.addMapping(LineSettings.AUTHORIZATION_ID, "port/P36");
            adapter.addMapping(LineSettings.USER_ID, "port/P35");            
            adapter.addMapping(LineSettings.PASSWORD, "port/P34");
            adapter.addMapping(LineSettings.DISPLAY_NAME, "port/P3");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "sip/P47");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        
        return impl;
    }
    
}
