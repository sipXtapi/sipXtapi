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

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.AbstractLine;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Polycom business functions for line meta setting
 */
public class PolycomLine extends AbstractLine {

    public static final String FACTORY_ID = "polycomLine";

    /** FIXME: Pull this value from Internal Dialing Rules */
    private static final String VOICEMAIL_EXT = "101";
    
    public Setting getSettingModel() {
        String systemDirectory = getPhoneContext().getSystemDirectory();
        File modelDefsFile = new File(systemDirectory + '/' 
                + PolycomPhone.FACTORY_ID + "/line.xml");
        Setting model = new XmlModelBuilder(systemDirectory).buildModel(modelDefsFile).copy();
        
        return model;
    }
    
    public Object getAdapter(Class interfac) {
        Object impl;
        if (interfac == LineSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(interfac);
            adapter.setSetting(getSettings());
            adapter.addMapping(LineSettings.AUTHORIZATION_ID, "reg/auth.userId");
            adapter.addMapping(LineSettings.USER_ID, "reg/address");            
            adapter.addMapping(LineSettings.PASSWORD, "reg/auth.password");
            adapter.addMapping(LineSettings.DISPLAY_NAME, "reg/displayName");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "reg/server/1/address");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER_PORT, "reg/server/1/port");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        
        return impl;
    }
    
    protected void setDefaults() {
        super.setDefaults();
        
        User u = getLineData().getUser();
        if (u != null) {
            PhoneDefaults defaults = getDefaults();
            Setting mwi = getSettings().getSetting("msg.mwi");
            String uri = u.getDisplayId() + '@' + defaults.getDomainName();
            mwi.getSetting("subscribe").setValue(uri);
            mwi.getSetting("callBack").setValue(VOICEMAIL_EXT + '@' + defaults.getDomainName());
            mwi.getSetting("callBackMode").setValue("contact");
        }        
    }
}
