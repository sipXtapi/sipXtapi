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
import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.common.PrimaryKeySource;
import org.sipfoundry.sipxconfig.setting.Folder;
import org.sipfoundry.sipxconfig.setting.ValueStorage;

/**
 * Database object representing an actualy physical phone you can touch.
 */
public class PhoneMetaData implements PrimaryKeySource, Serializable {
    
    public static final String FOLDER_RESOURCE_NAME = "endpoint";

    private static final long serialVersionUID = 1L;

    private Integer m_id = PhoneContext.UNSAVED_ID;

    private String m_name;

    private String m_serialNumber;

    private String m_factoryId;

    private ValueStorage m_valueStorage;

    private Folder m_folder;

    private List m_lines;
    
    private transient Phone m_phone;
    
    /** BEAN ACCESS ONLY **/
    public PhoneMetaData() {    
    }
    
    public PhoneMetaData(String factoryId) {
        setFactoryId(factoryId);
    }
    
    public Phone getPhone() {
        return m_phone;
    }
    
    public void setPhone(Phone phone) {
        m_phone = phone;
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

    public Integer getId() {
        return m_id;
    }

    public void setId(Integer id) {
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

    public Folder getFolder() {
        return m_folder;
    }

    public void setFolder(Folder folder) {
        m_folder = folder;
    }

    public List getLines() {
        return m_lines;
    }

    /**
     * Sets endpoint and position values on line. Safer way then
     * calling getLines().add(line) 
     */
    public void addLine(LineMetaData line) {
        List lines = getLines();
        if (lines == null) {
            lines = new ArrayList();
            setLines(lines);
        }
        line.setPhoneMetaData(this);
        line.setPosition(lines.size());
        lines.add(line);
    }
    
    /**
     * automatically set's the endpoint object and position
     * <pre>
     * Example: 
     * 
     * List lines = new ArrayList();
     * Line line = new Line();
     * line.setUser(user);
     * lines.add(line)
     * endpoint.setLines(lines);
     * phoneContext.storeEndpoint(endpoint);
     * </pre>
     */
    public void setLines(List lines) {
        m_lines = lines;
    }

    public Object getPrimaryKey() {
        return getId();
    }
}
