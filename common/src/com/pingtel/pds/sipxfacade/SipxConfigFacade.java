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
package com.pingtel.pds.sipxfacade;

/**
 * Comments
 */
public interface SipxConfigFacade {    
    
    public String getDeviceProfileName(int profileType, String vendor, 
            String model, String macAddress);

}
