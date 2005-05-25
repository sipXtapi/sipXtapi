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

    public static final String OPERATOR_ID = "operator";
    
    private static final String OPERATOR_IMPLIED_EXT = "0";
    
    private static final String OPERATOR_DEFAULT_EXT = "100";

    private static final String SYSTEM_NAME_PREFIX = "xcf";
    
    private static final String OPERATOR_DEFAULT_PROMPT = "welcome.wav";
    
    private String m_name;

    private String m_extension;

    private String m_description;

    private String m_prompt;
    
    private Map m_menuItems;
    
    private String m_systemId;
    
    public AutoAttendant() {        
    }
    
    public static AutoAttendant createOperator() {
        AutoAttendant operator = new AutoAttendant();
        operator.setSystemId(OPERATOR_ID);
        operator.setName("Operator");
        operator.setPrompt(OPERATOR_DEFAULT_PROMPT);
        operator.setExtension(OPERATOR_DEFAULT_EXT);
        operator.resetToFactoryDefault();
        
        return operator;
    }
    
    /**
     * This is the name passed to the mediaserver cgi to locate the correct auto attendant.
     * Technically it's invalid until saved to database.
     */
    public String getSystemName() {
        if (getSystemId() != null) {
            return getSystemId();            
        }
        return SYSTEM_NAME_PREFIX + getId().toString();
    }    
    
    /**
     * Certain auto attendants like the operator are system known. 
     * 
     * @return null if attendant is not system known
     */
    public String getSystemId() {
        return m_systemId;
    }    

    public void setSystemId(String systemId) {
        m_systemId = systemId;
    }
    
    public boolean isOperator() {
        return OPERATOR_ID.equals(getSystemId()); 
    }
    
    public String[] getDialingPatterns() {
        String[] patterns;
        if (isOperator()) {
            if (getExtension() != null) {
                patterns = new String[] { 
                    OPERATOR_ID, getExtension(), OPERATOR_IMPLIED_EXT 
                };
            } else {                
                patterns = new String[] { 
                    OPERATOR_ID, OPERATOR_IMPLIED_EXT 
                };
            }
        } else {
            if (getExtension() != null) {
                patterns = new String[] { 
                    getExtension()
                };
            } else {                
                patterns = new String[0];
            }            
        }
        
        return patterns;        
    }

    public String getDescription() {
        return m_description;
    }
    
    public String getScriptFileName() {
        return "autoattendant-" + getSystemName() + ".vxml";
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

    public void resetToFactoryDefault() {
        if (m_menuItems != null) {
            m_menuItems.clear();
        }
        setDescription(null);
        if (isOperator()) {
            addMenuItem(DialPad.NUM_0, new AttendantMenuItem(AttendantMenuAction.OPERATOR));
            addMenuItem(DialPad.NUM_9, new AttendantMenuItem(AttendantMenuAction.DIAL_BY_NAME));
            addMenuItem(DialPad.STAR, new AttendantMenuItem(AttendantMenuAction.REPEAT_PROMPT));
            addMenuItem(DialPad.POUND, new AttendantMenuItem(AttendantMenuAction.VOICEMAIL_LOGIN));
        } else {
            addMenuItem(DialPad.NUM_0, new AttendantMenuItem(AttendantMenuAction.OPERATOR));
            addMenuItem(DialPad.STAR, new AttendantMenuItem(AttendantMenuAction.REPEAT_PROMPT));            
        }
    }

}
