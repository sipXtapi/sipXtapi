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


/**
 * Let Models in MVC systems instantiate subclasses to abstract business
 * objects so they can add view specific properties. 
 */
public interface PhoneSummaryFactory {
    
    public PhoneSummary createPhoneSummary();
        
    public PhoneContext getPhoneContext();
}
