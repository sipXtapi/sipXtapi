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
package org.sipfoundry.sipxconfig.components;

import org.apache.hivemind.Messages;
import org.apache.tapestry.form.IPropertySelectionModel;

/**
 * Pull strings from an optional IMessages class, available from all components via
 * "messages"
 */
public class LocalizedOptionModelDecorator implements IPropertySelectionModel {
    
    private Messages m_messages;
    
    private String m_prefix;
    
    private IPropertySelectionModel m_model;
    
    public LocalizedOptionModelDecorator() {        
    }
    
    public LocalizedOptionModelDecorator(IPropertySelectionModel model, Messages messages, String prefix) {
        setModel(model);
        setMessages(messages);
        setResourcePrefix(prefix);
    }
        
    public void setResourcePrefix(String prefix) {
        m_prefix = prefix;
    }
    
    public void setMessages(Messages messages) {
        m_messages = messages;
    }

    public String getLabel(int index) {
        String rawLabel = m_model.getLabel(index);
        String label = TapestryUtils.localize(m_messages, m_prefix, rawLabel);
        
        return label;
    }
    
    public int getOptionCount() {
        return m_model.getOptionCount();
    }

    public Object getOption(int index) {
        return m_model.getOption(index);
    }

    public String getValue(int index) {
        return m_model.getValue(index);
    }

    public Object translateValue(String value) {
        return m_model.translateValue(value);
    }

    public void setModel(IPropertySelectionModel model) {
        m_model = model;
    }
}
