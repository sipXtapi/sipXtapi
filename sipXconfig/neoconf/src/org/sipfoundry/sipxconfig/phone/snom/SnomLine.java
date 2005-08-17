/*
 *
 *
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 snom technology AG
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.snom;

import java.io.File;

import org.sipfoundry.sipxconfig.phone.AbstractLine;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Snom business functions for line meta setting
 */
public class SnomLine extends AbstractLine {

    public static final String FACTORY_ID = "SnomLine";

    public SnomLine() {
        setModelFile("snom/line.xml");
    }

    public Setting getSettingModel() {
        File sysDir = new File(getPhoneContext().getSystemDirectory());
        File modelDefsFile = new File(sysDir, getModelFile());
        Setting model = new XmlModelBuilder(sysDir).buildModel(modelDefsFile);
        SnomPhone snomPhone = (SnomPhone) getPhone();
        return model.getSetting(snomPhone.getModel().getModelId());
    }

    public Object getAdpater(Class interfac) {
        Object impl;
        if (interfac == LineSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(interfac);
            adapter.setSetting(getSettings());
            adapter.addMapping(LineSettings.USER_ID, "line/user_name");
            adapter.addMapping(LineSettings.PASSWORD, "line/user_pass");
            adapter.addMapping(LineSettings.AUTHORIZATION_ID, "line/user_pname");
            adapter.addMapping(LineSettings.DISPLAY_NAME, "line/user_realname");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "line/user_host");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        return impl;
    }
}
