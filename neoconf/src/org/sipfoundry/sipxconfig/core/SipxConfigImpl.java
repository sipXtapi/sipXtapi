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
package org.sipfoundry.sipxconfig.core;


/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public class SipxConfigImpl implements SipxConfig {

    private CoreDao m_dao;
    
    private PhoneFactory m_phoneFactory;

    public CoreDao getCoreDao() {
        return m_dao;
    }

    public void setCoreDao(CoreDao dao) {
        m_dao = dao;
    }
    
    public PhoneFactory getPhoneFactory() {
        return m_phoneFactory;
    }
    
    public void setPhoneFactory(PhoneFactory phoneFactory) {
        m_phoneFactory = phoneFactory;
    }
}
