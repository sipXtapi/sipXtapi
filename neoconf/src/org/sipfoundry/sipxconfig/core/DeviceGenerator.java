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
 * Device specific support for inside ProfileGenerator. Details to follow
 */
public interface DeviceGenerator {
    
    /**
     * XML filename that describes a particular model's definitions
     * 
     * @return filepath to xml file
     */
    public String getDefinitions();

}
