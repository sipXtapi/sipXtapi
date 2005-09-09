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
package org.sipfoundry.sipxconfig.phone.cisco;

import org.sipfoundry.sipxconfig.phone.Phone;

/**
 * Support for Cisco 7940/7960
 */
public abstract class CiscoPhone extends Phone {

    public static final String PORT = "port";
    
    public static final String SIP = "sip";

    private String m_phoneTemplate;
    
    protected CiscoPhone(String beanId) {
        super(beanId);
    }
    
    protected CiscoPhone(CiscoModel model) {
        super(model);
    }

    public abstract String getPhoneFilename();

    public String getPhoneTemplate() {
        return m_phoneTemplate;
    }

    public void setPhoneTemplate(String phoneTemplate) {
        m_phoneTemplate = phoneTemplate;
    }

    public int getMaxLineCount() {
        return getModel().getMaxLineCount();
    }
    
    public void restart() {
        sendCheckSyncToFirstLine();        
    }
}
