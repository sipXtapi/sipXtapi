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
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingGroup;
import org.sipfoundry.sipxconfig.setting.ValueStorage;

/**
 * Database object representing an actualy physical phone you can touch.
 */
public class Endpoint implements PrimaryKeySource, Serializable {
    
    public static final String FOLDER_RESOURCE_NAME = "endpoint";

    private static final long serialVersionUID = 1L;

    private int m_id = PhoneContext.UNSAVED_ID;

    private String m_name;

    private String m_serialNumber;

    private String m_phoneId;

    private ValueStorage m_valueStorage;

    private Folder m_folder;

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

    public Folder getFolder() {
        return m_folder;
    }

    public void setFolder(Folder folder) {
        m_folder = folder;
    }

    public Setting getSettings(Phone phone) {
        Setting settings = phone.getSettingModel(this);
        if (m_valueStorage == null) {
            m_valueStorage = new ValueStorage();
        }
        
        if (m_folder != null) {
            settings = (SettingGroup) m_folder.decorate(settings);
        }
        
        return (SettingGroup) m_valueStorage.decorate(settings);
    }

    public List getLines() {
        return m_lines;
    }

    /**
     * Sets endpoint and position values on line. Safer way then
     * calling getLines().add(line) 
     */
    public void addLine(Line line) {
        List lines = getLines();
        if (lines == null) {
            lines = new ArrayList();
            setLines(lines);
        }
        line.setEndpoint(this);
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
        return new Integer(getId());
    }
}