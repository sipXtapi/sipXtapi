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

import java.io.Serializable;
import java.util.List;

import org.sipfoundry.sipxconfig.setting.ValueStorage;


/**
 * Database object representing an actualy physical phone you can touch.
 */
public class Endpoint implements Serializable {

    private static final long serialVersionUID = 1L;

    private int m_id = PhoneContext.UNSAVED_ID;

    private String m_name;

    private String m_serialNumber;

    private String m_phoneId;
    
    private ValueStorage m_valueStorage;
    
    private List m_lines;

    /**
     * @return ids used in PhoneFactory
     */
    public String getPhoneId() {
        return m_phoneId;
    }

    /**
     * @param phoneId used in PhoneFactory
     */
    public void setPhoneId(String phoneId) {
        m_phoneId = phoneId;
    }

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getSerialNumber() {
        return m_serialNumber;
    }

    public void setSerialNumber(String serialNumber) {
        m_serialNumber = serialNumber;
    }
    
    /**
     * @return name if set otherwise serial number, convienent for display purposes
     */
    public String getDisplayLabel() {
        return m_name != null ? m_name : m_serialNumber;
    }
    
    public ValueStorage getValueStorage() {
        return m_valueStorage;
    }
    
    public void setValueStorage(ValueStorage valueStorage) {
        m_valueStorage = valueStorage;
    }
    
    public List getLines() {
        return m_lines;
    }
    
    public void setLines(List lines) {
        m_lines = lines;
    }
}
