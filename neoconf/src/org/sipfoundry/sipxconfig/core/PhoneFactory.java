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

import org.springframework.beans.factory.BeanFactory;

/**
 * Comments
 */
public interface PhoneFactory extends BeanFactory {
    
    public Phone getPhoneByModel(String model);

    public Phone getPhoneByVendor(String model);
    
}
