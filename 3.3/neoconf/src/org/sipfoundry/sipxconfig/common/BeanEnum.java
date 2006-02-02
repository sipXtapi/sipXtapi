/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.enums.Enum;

/**
 * Capture static details about a bean. Examples: types of phones, types of uploads
 */
public abstract class BeanEnum extends Enum {

    private String m_beanId;
    
    private String m_label;
    
    private String m_enumId;
        
    private String[] m_details;
    
    public BeanEnum(String beanId, String label) {
        this(beanId, StringUtils.EMPTY, label);
    }
    
    public BeanEnum(String beanId, String enumId, String label) {
        this(beanId, enumId, null, label);
    }

    public BeanEnum(String beanId, String enumId, String[] details, String label) {
        super(beanId + enumId + StringUtils.defaultString(StringUtils.join(details)));
        m_beanId = beanId;
        m_enumId = enumId;
        m_details = details;
        m_label = label;        
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
    
    protected String getEnumId() {
        return m_enumId;
    }
    
    /**
     * Additional details about this enum, version, build, etc.
     */
    public String[] getDetails() {
        return m_details;
    }
}
