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

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingGroup;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000 conference phone
 */
public class PolycomPhone extends GenericPhone {

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
            File f = new File(tftpRoot + outputFile);
            File d = f.getParentFile();
            if (!d.exists()) {
                if (!d.mkdirs()) {
                    throw new RuntimeException("Could not create profile directory "
                            + d.getPath());
                }
            }
            out = new FileWriter(f);
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
        generateProfile(core, app.getCoreFilename());

        PhoneConfiguration phone = new PhoneConfiguration(this, endpoint);
        phone.setTemplate(config.getPhoneTemplate());
        generateProfile(phone, app.getPhoneFilename());

        SipConfiguration sip = new SipConfiguration(this, endpoint);
        sip.setTemplate(config.getSipTemplate());
        generateProfile(sip, app.getSipFilename());

        app.deleteStaleDirectories();
    }

    public SettingGroup getSettingModel(Line line) {

        // set default values to current environment settings
        SettingGroup lineModel = super.getSettingModel(line);
        User u = line.getUser();
        if (u != null) {
            SettingGroup reg = (SettingGroup) lineModel
                    .getSetting(ConfigurationTemplate.REGISTRATION_SETTINGS);
            reg.getSetting("displayName").setValue(u.getDisplayId());
        }

        // See pg. 125 Admin Guide/16 June 2004
        if (line.getPosition() == 0) {
            lineModel.getSetting("msg.mwi").getSetting("callBackMode").setValue("registration");
        }

        return lineModel;
    }
}

