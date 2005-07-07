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

import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

public class KPhone extends GenericPhone {
    
    public static final String FACTORY_ID = "kphone";
    
    public KPhone() {
        // Tells superclass what bean to create for lines
        setLineFactoryId(KPhoneLine.FACTORY_ID);
        setPhoneTemplate(FACTORY_ID + "/kphonerc.vm");
    }
    
    public Setting getSettingModel() {
        String systemDirectory = getPhoneContext().getSystemDirectory();
        File modelDefsFile = new File(systemDirectory + '/' + FACTORY_ID + "/phone.xml");
        Setting model = new XmlModelBuilder(systemDirectory).buildModel(modelDefsFile).copy();
        
        return model;
    }

    public String getPhoneFilename() {
        return getWebDirectory() + "/" + getPhoneData().getSerialNumber() + ".kphonerc";
    }    
}
