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

import org.sipfoundry.sipxconfig.common.DataCollectionItem;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Folder;
import org.sipfoundry.sipxconfig.setting.ValueStorage;

/**
 * Association between Users and their assigned phones.
 */
public class LineMetaData implements Serializable, DataCollectionItem {

    public static final String FOLDER_RESOURCE_NAME = "line";

    private static final long serialVersionUID = 1L;

    private Integer m_id = PhoneContext.UNSAVED_ID;

    private User m_user;

    private ValueStorage m_valueStorage;

    private Folder m_folder;

    private PhoneMetaData m_endpoint;

    private int m_position;
    
    private transient Line m_line;

    public Line getLine() {
        return m_line;
    }
    
    public void setLine(Line line) {
        m_line = line;
    }

    public Integer getId() {
        return m_id;
    }

    public void setId(Integer id) {
        m_id = id;
    }

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public Folder getFolder() {
        return m_folder;
    }

    public void setFolder(Folder folder) {
        m_folder = folder;
    }

    public ValueStorage getValueStorage() {
        return m_valueStorage;
    }

    public void setValueStorage(ValueStorage valueStorage) {
        m_valueStorage = valueStorage;
    }

    public String getDisplayLabel() {
        return m_user.getDisplayId();
    }

    public PhoneMetaData getPhoneMetaData() {
        return m_endpoint;
    }

    public void setPhoneMetaData(PhoneMetaData endpoint) {
        m_endpoint = endpoint;
    }
    
    public Object getPrimaryKey() {
        return m_id;
    }

    public int getPosition() {
        return m_position;
    }

    public void setPosition(int position) {
        m_position = position;
    }
}
