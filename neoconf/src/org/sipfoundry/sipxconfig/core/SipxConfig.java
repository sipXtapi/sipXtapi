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
public interface SipxConfig {

    public CoreDao getCoreDao();

    public void setCoreDao(CoreDao dao);
        
    public PhoneFactory getPhoneFactory();
    
    public void setPhoneFactory(PhoneFactory phoneFactory);
}
