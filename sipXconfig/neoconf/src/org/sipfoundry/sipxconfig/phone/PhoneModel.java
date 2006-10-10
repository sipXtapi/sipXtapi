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

import org.sipfoundry.sipxconfig.device.DeviceDescriptor;

public class PhoneModel extends DeviceDescriptor {

    private static final int DEFAULT_MAX_LINES = 4;

    private int m_maxLineCount = DEFAULT_MAX_LINES;
    
    public PhoneModel() {        
    }
    
    public PhoneModel(String beanId) {
        super(beanId);
    }
    
    public PhoneModel(String beanId, String modelId) {
        super(beanId, modelId);
    }

    public void setMaxLineCount(int maxLineCount) {
        m_maxLineCount = maxLineCount;
    }

    public int getMaxLineCount() {
        return m_maxLineCount;
    }
}
