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

import java.io.IOException;



/**
 * Implement this to add support for new devices to the system
 */
public interface Phone {

    public String getModelId();
    
    public void setModelId(String id);
    
    public String getDisplayLabel();

    public void setEndpoint(Endpoint endpoint);
    
    public Endpoint getEndpoint();

    public void generateProfiles(PhoneContext phoneContext) throws IOException;
    
    public void restart(PhoneContext phoneContext) throws IOException;
}
