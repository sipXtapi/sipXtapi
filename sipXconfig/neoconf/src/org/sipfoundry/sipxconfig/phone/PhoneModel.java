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
package org.sipfoundry.sipxconfig.phone;

import java.util.LinkedHashMap;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.enums.Enum;


public class PhoneModel extends Enum {
    
    private static final Map REGISTERED = new LinkedHashMap();

    private String m_beanId;
    
    private String m_label;
    
    private String m_modelId;
    
    private int m_maxLineCount;
        
    public PhoneModel(String beanId, String label) {
        this(beanId, StringUtils.EMPTY, label);
    }
    
    public PhoneModel(String beanId, String modelId, String label) {
        this(beanId, modelId, label, 1);
    }
    
    public PhoneModel(String beanId, String modelId, String label, int maxLineCount) {
        super(beanId + modelId);
        m_beanId = beanId;
        m_modelId = modelId;
        m_label = label;        
        m_maxLineCount = maxLineCount;
        setRegistered(true);
    }
    
    /**
     * Spring bean name, NOTE: You cannot reuse java class to multiple spring beans. One class/bean
     * but a spring bean can handle multiple models
     */
    public String getBeanId() {
        return m_beanId;
    }
    
    /**
     * User identifiable label for this model
     */
    public String getLabel() {
        return m_label;
    } 
    
    public String getModelId() {
        return m_modelId;
    }

    public int getMaxLineCount() {
        return m_maxLineCount;
    }
    
    /**
     * @return PhoneModel that corresponds to beanId + modelId unique identifier
     */
    public static PhoneModel getModel(String beanId, String modelId) {
        return getModel(beanId + modelId);
    }
    
    public static PhoneModel getModel(String name) {
        PhoneModel model = (PhoneModel) REGISTERED.get(name);
        if (model == null) {            
            throw new IllegalArgumentException("Unknown model, model name = " + name);
        }                   
        return model;
    }
    
    public void setRegistered(boolean registered) {
        if (registered) {
            REGISTERED.put(getName(), this);
        } else {
            REGISTERED.remove(getName());
        }
    }
}
