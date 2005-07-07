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

import org.sipfoundry.sipxconfig.phone.AbstractLine;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Cisco business functions for line meta setting
 */
public abstract class CiscoLine extends AbstractLine {

    public Setting getSettingModel() {
        File sysDir = new File(getPhoneContext().getSystemDirectory());
        File modelDefsFile = new File(sysDir, getModelFile());
        Setting model = new XmlModelBuilder(sysDir).buildModel(modelDefsFile);
        
        CiscoPhone ciscoPhone = (CiscoPhone) getPhone();
        return model.getSetting(ciscoPhone.getModel().getModelId());
    }
}
