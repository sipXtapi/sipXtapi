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
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;

import org.apache.velocity.Template;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.phone.Organization;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingModel;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000 conference phone
 */
public class PolycomPhone extends GenericPhone {

    /** basic model */
    public static final String MODEL_300 = "polycom300";

    /** standard model */
    public static final String MODEL_500 = "polycom500";

    /** deluxe model */
    public static final String MODEL_600 = "polycom600";

    /** conference phone */
    public static final String MODEL_3000 = "polycom3000";

    private String m_id;

    private String m_tftpRoot;

    private String m_systemDirectory;

    private String m_modelDefinitions = "polycom/model.xml";

    private String m_phoneConfigTemplate = "polycom/phone1.cfg";

    private VelocityEngine m_velocityEngine;

    private SettingModel m_settingModel;

    public VelocityEngine getVelocityEngine() {
        return m_velocityEngine;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public String getTftpRoot() {
        return m_tftpRoot;
    }

    public void setTftpRoot(String tftpRoot) {
        m_tftpRoot = tftpRoot;
    }

    public String getModelId() {
        return m_id;
    }

    public String getDisplayLabel() {
        return "Polycom SoundPoint IP 300";
    }

    public void setModelId(String id) {
        m_id = id;
    }

    public SettingModel getSettingModel() {
        if (m_settingModel == null) {
            FileInputStream is = null;
            try {
                File modelDefsFile = getFile(getSystemDirectory(), getModelDefinitions());
                is = new FileInputStream(modelDefsFile);
                XmlModelBuilder build = new XmlModelBuilder();
                m_settingModel = build.buildModel(is);

            } catch (IOException e) {
                throw new RuntimeException("Cannot parse polycom model definitions file "
                        + getModelDefinitions(), e);
            } finally {
                if (is != null) {
                    try {
                        is.close();
                    } catch (IOException ignore) {
                        // cleanup non fatal
                        log(ignore);
                    }
                }
            }
        }

        return m_settingModel;
    }

    private void log(Exception e) {
        e.printStackTrace();
    }

    private void initialize(PhoneContext context) {
        SettingSet settings = getEndpoint().getSettings();
        Organization org = context.loadRootOrganization();

        settings.setDefault("network/tftpServer", org.getDnsDomain());
        File tftpRootFile = new File(getTftpRoot());
        if (!tftpRootFile.exists()) {
            if (!tftpRootFile.mkdirs()) {
                throw new RuntimeException("Could not create TFTP root directory "
                        + tftpRootFile.getPath());
            }
        }
    }

    public void generateProfiles(PhoneContext context) throws IOException {
        initialize(context);
        Template template;
        // has to be relative to system directory
        String templateFile = getPhoneConfigTemplate();
        try {
            template = getVelocityEngine().getTemplate(templateFile);
        } catch (Exception e) {
            throw new RuntimeException("Error creating velocity template " 
                    + templateFile, e);
        }

        // PERFORMANCE: depending on how resource intensive the above code is
        // try to reuse the template objects for subsequent profile
        // generations

        VelocityContext velocityContext = new VelocityContext();
        velocityContext.put("phone", this);

        FileWriter wtr = null;
        File profile = getFile(getTftpRoot(), getPhoneConfigFilename());
        try {
            wtr = new FileWriter(profile);
            template.merge(velocityContext, wtr);
        } catch (Exception e) {
            throw new RuntimeException("Error using velocity template " 
                    + templateFile + " to create output profile "
                    + profile.getPath(), e);
        } finally {
            if (wtr != null) {
                wtr.close();
            }
        }
    }

    public String getPhoneConfigFilename() {
        // TODO: if using TFTP, have to put sequence number into config file
        // per polycom documentation
        return getEndpoint().getSerialNumber() + ".cfg";
    }

    public String getModelDefinitions() {
        return m_modelDefinitions;
    }

    public void setModelDefinitions(String modelDefinitions) {
        m_modelDefinitions = modelDefinitions;
    }

    public InputStream getPhoneConfigFile() throws IOException {
        return new FileInputStream(getFile(m_tftpRoot, getPhoneConfigFilename()));
    }

    public String getPhoneConfigTemplate() {
        return m_phoneConfigTemplate;
    }

    public void setPhoneConfigTemplate(String phoneConfigTemplate) {
        m_phoneConfigTemplate = phoneConfigTemplate;
    }

    public String getSystemDirectory() {
        return m_systemDirectory;
    }

    public void setSystemDirectory(String systemDirectory) {
        m_systemDirectory = systemDirectory;
    }
    
    File getFile(String root, String filename) {
        return new File(root + '/' + filename);
    }
}
