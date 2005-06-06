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
package org.sipfoundry.sipxconfig.phone.cisco;

import java.io.File;

import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Support for Cisco 7940/7960
 */
public abstract class CiscoPhone extends GenericPhone {

    public static final String FACTORY_ID = "cisco";

    public static final String PORT = "port";
    
    public static final String SIP = "sip";

    private String m_phoneTemplate;

    public Setting getSettingModel() {
        File sysDir = new File(getPhoneContext().getSystemDirectory());
        File modelDefsFile = new File(sysDir, getModelFile());
        Setting all = new XmlModelBuilder(sysDir).buildModel(modelDefsFile);
        Setting model = all.getSetting(getModel().getModelId());
        
        return model;
    }
    
    public CiscoModel getModel() {
        return CiscoModel.getModel(getPhoneData().getFactoryId());
    }

    public abstract String getPhoneFilename();

    public String getPhoneTemplate() {
        return m_phoneTemplate;
    }

    public void setPhoneTemplate(String phoneTemplate) {
        m_phoneTemplate = phoneTemplate;
    }

    public int getMaxLineCount() {
        return getModel().getMaxLines();
    }
}
