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
package org.sipfoundry.sipxconfig.vendor;

import java.io.File;
import java.io.IOException;

import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

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

    public void generateProfiles(PhoneContext context_, Endpoint endpoint) throws IOException {
        initialize();
        
        PolycomPhoneConfig config = getConfig();
        
        ApplicationConfiguration app = new ApplicationConfiguration(this, endpoint);
        app.setTemplateFilename(config.getApplicationTemplate());
        app.generateProfile();        
        
        CoreConfiguration core = new CoreConfiguration(this, endpoint);
        core.setTemplateFilename(config.getCoreTemplate());
        core.setOutputFilename(app.getCoreFilename());
        core.generateProfile();
        
        PhoneConfiguration phone = new PhoneConfiguration(this, endpoint);
        phone.setTemplateFilename(config.getPhoneTemplate());
        phone.setOutputFilename(app.getPhoneFilename());
        phone.generateProfile();
        
        SipConfiguration sip = new SipConfiguration(this, endpoint);
        sip.setTemplateFilename(config.getSipTemplate());
        sip.setOutputFilename(app.getSipFilename());
        sip.generateProfile();        
    }
}
