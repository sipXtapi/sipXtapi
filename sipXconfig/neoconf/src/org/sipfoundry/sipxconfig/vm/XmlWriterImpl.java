/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.Writer;

import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;

/**
 * Helper class for reading/writing XML
 */
public abstract class XmlWriterImpl<T> {  
    private String m_template;    
    private VelocityEngine m_velocityEngine;

    public String getTemplate() {
        return m_template;
    }

    public void setTemplate(String template) {
        m_template = template;
    }

    public void writeObject(T object, Writer output) {        
        VelocityContext velocityContext = new VelocityContext();
        addContext(velocityContext, object);
        writeObject(velocityContext, object, output);
    }
    
    protected abstract void addContext(VelocityContext context, T object);
    
    protected void writeObject(VelocityContext context, T object, Writer output) {
        try {
            getVelocityEngine().mergeTemplate(getTemplate(), context, output);
        } catch (Exception e) {
            throw new RuntimeException("Error using velocity template " + getTemplate(), e);
        }        
    }

    public VelocityEngine getVelocityEngine() {
        return m_velocityEngine;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }
}
