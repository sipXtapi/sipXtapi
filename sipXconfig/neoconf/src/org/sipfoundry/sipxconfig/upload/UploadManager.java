/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.upload;

import java.util.List;

import org.sipfoundry.sipxconfig.common.DataObjectSource;

public interface UploadManager extends DataObjectSource {
    
    public static final String CONTEXT_BEAN_NAME = "uploadManager";
    
    public Firmware loadFirmware(Integer firmwareId);
    
    public void saveFirmware(Firmware firmware);
    
    public void deleteFirmware(Firmware firmware);
    
    public List getFirmwareManufacturers();

    public List getFirmware();
    
    public Firmware newFirmware(FirmwareManufacturer manufacturer);

}
