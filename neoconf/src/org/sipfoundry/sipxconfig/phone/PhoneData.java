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


/**
 * Database object representing an actualy physical phone you can touch.
 */
public class PhoneData extends AbstractData {
    
    private String m_name;

    private String m_serialNumber;

    private String m_factoryId;
    
    private String m_modelLabel;

    /** BEAN ACCESS ONLY **/
    public PhoneData() {    
    }
    
    public PhoneData(String factoryId) {
        setFactoryId(factoryId);
        setModelLabel(factoryId);
    }
    
    /**
     * @return ids used in PhoneFactory
     */
    public String getFactoryId() {
        return m_factoryId;
    }

    /**
     * @param phoneId used in PhoneFactory
     */
    public void setFactoryId(String phoneId) {
        m_factoryId = phoneId;
    }

    public String getModelLabel() {
        return m_modelLabel;
    }

    public void setModelLabel(String modelLabel) {
        m_modelLabel = modelLabel;
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
        m_serialNumber = cleanSerialNumber(serialNumber);
    }
    
    private static String cleanSerialNumber(String rawNumber) {
        String clean = rawNumber.toLowerCase();
        clean = clean.replaceAll("[:\\s]*", "");
        
        return clean;        
    }

    /**
     * @return name if set otherwise serial number, convienent for display purposes
     */
    public String getDisplayLabel() {
        return m_name != null ? m_name : m_serialNumber;
    }
}
