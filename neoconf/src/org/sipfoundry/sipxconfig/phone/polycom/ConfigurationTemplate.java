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

import java.io.Writer;

import org.apache.velocity.Template;
import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.phone.Endpoint;

/**
 * Baseclass for velocity template generators
 */
public abstract class ConfigurationTemplate {
    
    public static final String CALL_SETTINGS = "call";

    public static final String REGISTRATION_SETTINGS = "reg";

    private PolycomPhone m_phone;

    private Endpoint m_endpoint;

    private String m_template;

    public ConfigurationTemplate(PolycomPhone phone, Endpoint endpoint) {
        m_phone = phone;
        m_endpoint = endpoint;
    }

    public String getTemplate() {
        return m_template;
    }

    public void setTemplate(String template) {
        m_template = template;
    }

    public Endpoint getEndpoint() {
        return m_endpoint;
    }

    public PolycomPhone getPhone() {
        return m_phone;
    }

    public abstract void addContext(VelocityContext context);

    public void generateProfile(Writer out) {
        Template template;
        // has to be relative to system directory
        PolycomPhoneConfig config = getPhone().getConfig();
        try {
            template = config.getVelocityEngine().getTemplate(getTemplate());
        } catch (Exception e) {
            throw new RuntimeException("Error creating velocity template " + getTemplate(), e);
        }

        // PERFORMANCE: depending on how resource intensive the above code is
        // try to reuse the template objects for subsequent profile
        // generations

        VelocityContext velocityContext = new VelocityContext();
        velocityContext.put("phone", getPhone());
        velocityContext.put("endpoint", getEndpoint());
        addContext(velocityContext);

        try {
            template.merge(velocityContext, out);
        } catch (Exception e) {
            throw new RuntimeException("Error using velocity template " + getTemplate(), e);
        }
    }
}
