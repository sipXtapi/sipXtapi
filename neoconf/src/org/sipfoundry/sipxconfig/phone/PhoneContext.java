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
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public interface PhoneContext {

    public PhoneDao getPhoneDao();

    public void setPhoneDao(PhoneDao dao);
        
    public Phone getPhone(Endpoint endpoint);
}
