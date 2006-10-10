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
package org.sipfoundry.sipxconfig.phone.grandstream;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

/**
 * Static differences in grandstream models
 */
public final class GrandstreamModel extends PhoneModel {

    private boolean m_isHandyTone;
    
    public GrandstreamModel() {
        setBeanId(GrandstreamPhone.BEAN_ID);
    }
    
    public void setIsHandyTone(boolean isHandyTone) {
        m_isHandyTone = isHandyTone;
    }

    public boolean isHandyTone() {
        return m_isHandyTone;
    }
}
