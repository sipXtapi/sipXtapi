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

import java.util.List;



/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public interface PhoneContext {
    
    public static final String CONTEXT_BEAN_NAME = "phoneContext";
    
    public PhoneDao getPhoneDao();

    public void setPhoneDao(PhoneDao dao);
        
    public Phone getPhone(Endpoint endpoint);
    
    public Phone getPhone(int endpointId);
    
    public List getPhoneIds();

    public void setPhoneIds(List phoneIds);
}
