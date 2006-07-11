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
package org.sipfoundry.sipxconfig.components;

/**
 * Helper for managing navigational tabs, specifically what tab should
 * be visible and what css class attribute should be displayed. 
 * <pre>
 * Example Tapestry *.page entry:
 *   &lt;bean name="tabs" class="org.sipfoundry.sipxconfig.components.Tabs"&gt;
 *       &lt;property name="activeTab" value="tab"/&gt;
 *   &lt;/bean&gt;
 *   &lt;component id="deviceIdTab" type="ActionLink"&gt;
 *       &lt;static-binding name="listener" value="listeners.editPhone"/&gt;
 *       &lt;binding name="class" expression="beans.tabs.cssClass('EditPhone')"/&gt;
 *   &lt;/component&gt;
 * </pre>
 */
public class Tabs {
    
    static final String ACTIVE_CLASS = "tab active";
    
    static final String INACTIVE_CLASS = "tab";
    
    private String m_activeTabId;
    
    public String getTabClass(String tabId) {
        return tabId != null && tabId.equals(m_activeTabId) ? ACTIVE_CLASS : INACTIVE_CLASS;
    }
    
    public void setActiveTab(String tabId) {
        m_activeTabId = tabId;
    }
}
