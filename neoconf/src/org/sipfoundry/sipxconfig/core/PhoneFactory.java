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
 * Contruct abstract Phone objects from concrete implementations
 * provided by vendors.  Phones can normally register their
 * phone in PhoneFactory.xml  
 */
public interface PhoneFactory {
    
    public Phone getPhoneByModel(String model);

    public Phone getPhoneByVendor(String model);
    
    public Phone getPhoneById(String id);
}
