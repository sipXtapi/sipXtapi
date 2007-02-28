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
package org.sipfoundry.sipxconfig.device;

import java.io.Writer;
import java.util.Map;
import java.util.Set;

import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;

/**
 * Baseclass for velocity template generators
 */
public class VelocityProfileGenerator extends AbstractProfileGenerator {
    private VelocityEngine m_velocityEngine;

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    @Override
    protected void generateProfile(ProfileContext context, String template, Writer out) {
        VelocityContext velocityContext = new VelocityContext();
        Set<Map.Entry<String, Object>> entries = context.getContext().entrySet();
        for (Map.Entry<String, Object> entry : entries) {
            velocityContext.put(entry.getKey(), entry.getValue());
        }

        try {
            m_velocityEngine.mergeTemplate(template, velocityContext, out);
        } catch (Exception e) {
            throw new RuntimeException("Error using velocity template " + template, e);
        }
    }
}
