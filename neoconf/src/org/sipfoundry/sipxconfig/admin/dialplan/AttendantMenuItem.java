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


/**
 * This represent the expected response of the auto attendant after user presses a dial-pad key
 */
public class AttendantMenuItem {
    
    private AttendantMenuAction m_action;

    private String m_parameter;
    
    /** BEAN ACCESS */
    public AttendantMenuItem() {
    }
    
    public AttendantMenuItem(AttendantMenuAction action) {
        setAction(action);
    }
    
    public AttendantMenuItem(AttendantMenuAction action, String parameter) {
        this(action);
        setParameter(parameter);
    }

    public AttendantMenuAction getAction() {
        return m_action;
    }

    public void setAction(AttendantMenuAction action) {
        m_action = action;
    }
    
    public String getActionId() {
        return m_action.getName();
    }

    /**
     * BEAN ACCESS ONLY, looks up action enumerated type
     */
    public void setActionId(String actionId) {
        m_action = AttendantMenuAction.getActionFromId(actionId);
    }    

    /**
     * Depends on the operation type, but this could be extension or aa menu name
     */
    public String getParameter() {
        return m_parameter;
    }

    public void setParameter(String parameter) {
        m_parameter = parameter;
    }
}
