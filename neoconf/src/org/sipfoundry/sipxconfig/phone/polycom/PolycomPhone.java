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
import java.io.FileWriter;
import java.io.IOException;

import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.User;
import org.sipfoundry.sipxconfig.setting.SettingGroup;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000 conference phone
 */
public class PolycomPhone extends GenericPhone {

    public static final String REGISTRATION_SETTINGS = "reg";

    private Polycom m_model = Polycom.MODEL_300;
    
    private PolycomPhoneConfig m_config;
    
    public PolycomPhone() {
        setEndpointModelFilename("polycom/phone.xml");
        setLineModelFilename("polycom/line.xml");        
    }
    
    public void setConfig(PolycomPhoneConfig config) {
        m_config = config;
    }
    
    public PolycomPhoneConfig getConfig() {
        return m_config;
    }

    public String getModelId() {
        return m_model.getModelId();
    }

    public String getDisplayLabel() {
        return m_model.getDisplayLabel();
    }

    public void setModelId(String id) {
        m_model = Polycom.getModel(id);
    }

    public int getMaxLineCount() {
        return m_model.getMaxLines();
    }

    private void initialize() {
        File tftpRootFile = new File(getConfig().getTftpRoot());
        if (!tftpRootFile.exists()) {
            if (!tftpRootFile.mkdirs()) {
                throw new RuntimeException("Could not create TFTP root directory "
                        + tftpRootFile.getPath());
            }
        }
    }

    /**
     * TODO: should be private, avoiding checkstyle error
     */
    void generateProfile(ConfigurationTemplate cfg, String outputFile) throws IOException {
        FileWriter out = null;                
        String tftpRoot = getConfig().getTftpRoot() + '/';
        try {
            out = new FileWriter(tftpRoot + outputFile);
            cfg.generateProfile(out);     
        } finally {
            if (out != null) {
                out.close();
            }
        }       
    }

    public void generateProfiles(PhoneContext context_, Endpoint endpoint) throws IOException {
        initialize();
        
        PolycomPhoneConfig config = getConfig();
        
        ApplicationConfiguration app = new ApplicationConfiguration(this, endpoint);
        app.setTemplate(config.getApplicationTemplate());
        generateProfile(app, app.getAppFilename());
        
        CoreConfiguration core = new CoreConfiguration(this, endpoint);
        core.setTemplate(config.getCoreTemplate());
        generateProfile(app, app.getCoreFilename());
        
        PhoneConfiguration phone = new PhoneConfiguration(this, endpoint);
        phone.setTemplate(config.getPhoneTemplate());
        generateProfile(app, app.getPhoneFilename());
        
        SipConfiguration sip = new SipConfiguration(this, endpoint);
        sip.setTemplate(config.getSipTemplate());
        generateProfile(app, app.getSipFilename());
    }
        
    public SettingGroup getSettingModel(Line line) {

        // set default values to current environment settings
        SettingGroup lineModel = super.getSettingModel(line);
        User u = line.getUser();
        if (u != null) {
            SettingGroup reg = (SettingGroup) lineModel.getSetting(REGISTRATION_SETTINGS);
            reg.getSetting("displayName").setDefaultValue(u.getDisplayId());
        }

        return lineModel;
    }
    
}
