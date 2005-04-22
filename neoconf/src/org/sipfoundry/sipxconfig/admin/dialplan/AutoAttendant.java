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
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Map;
import java.util.TreeMap;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.DialPad;

public class AutoAttendant extends BeanWithId {

    private String m_name;

    private String m_extension;

    private String m_description;

    private String m_prompt;
    
    private Map m_menuItems;

    public String getDescription() {
        return m_description;
    }
    
    public String getScriptFileName() {
        return "autoattendant-" + getId() + ".vxml";
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }

    public String getPrompt() {
        return m_prompt;
    }

    public void setPrompt(String prompt) {
        m_prompt = prompt;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    /**
     * @return map of AttendantMenuItems where the dialpad keys DialPad objects 
     * representing keys 0-9,* and #
     */
    public Map getMenuItems() {
        return m_menuItems;
    }    

    public void setMenuItems(Map menuItems) {
        m_menuItems = menuItems;
    }
    
    public void addMenuItem(DialPad key, AttendantMenuItem menuItem) {
        if (m_menuItems == null) {
            m_menuItems = new TreeMap();
        }
        
        m_menuItems.put(key, menuItem);
    }
}
