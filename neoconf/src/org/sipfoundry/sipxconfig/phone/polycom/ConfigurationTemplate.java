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

import org.apache.velocity.Template;
import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.phone.Endpoint;

/**
 * Baseclass for velocity template generators
 */
public abstract class ConfigurationTemplate {

    private PolycomPhone m_phone;

    private Endpoint m_endpoint;

    private String m_templateFilename;
    
    private String m_outputFilename;

    public ConfigurationTemplate(PolycomPhone phone, Endpoint endpoint) {
        m_phone = phone;
        m_endpoint = endpoint;
    }

    public String getTemplateFilename() {
        return m_templateFilename;
    }

    public Endpoint getEndpoint() {
        return m_endpoint;
    }

    public PolycomPhone getPhone() {
        return m_phone;
    }

    public void setOutputFilename(String outputFilename) {
        m_outputFilename = outputFilename;
    }
    
    public String getOutputFilename() {
        return m_outputFilename;
    }

    public void setTemplateFilename(String templateFilename) {
        m_templateFilename = templateFilename;
    }

    public abstract void addContext(VelocityContext context);

    public void generateProfile() throws IOException {
        Template template;
        // has to be relative to system directory
        PolycomPhoneConfig config = getPhone().getConfig();
        String templateFile = getTemplateFilename();
        try {
            template = config.getVelocityEngine().getTemplate(templateFile);
        } catch (Exception e) {
            throw new RuntimeException("Error creating velocity template " + templateFile, e);
        }

        // PERFORMANCE: depending on how resource intensive the above code is
        // try to reuse the template objects for subsequent profile
        // generations

        VelocityContext velocityContext = new VelocityContext();
        velocityContext.put("phone", getPhone());
        velocityContext.put("endpoint", getEndpoint());
        addContext(velocityContext);

        FileWriter wtr = null;
        File tftpRoot = new File(config.getTftpRoot());
        File profile = new File(tftpRoot, getOutputFilename());
        profile.getParentFile().mkdirs();
        try {
            wtr = new FileWriter(profile);
            template.merge(velocityContext, wtr);
        } catch (Exception e) {
            throw new RuntimeException("Error using velocity template " + templateFile
                    + " to create output profile " + profile.getPath(), e);
        } finally {
            if (wtr != null) {
                wtr.close();
            }
        }
    }
}
