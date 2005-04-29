/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.profilewriter;


public class InstallDeviceSoftware extends ProfileWriterCommand {
    /**
     * 
     * @param macAddress
     * @param deviceURL
     * @param model
     * @param currentVersion
     * @param targetVersion
     */
    public InstallDeviceSoftware( String macAddress,
                                  String deviceURL,
                                  String model,
                                  String currentVersion,
                                  String targetVersion ) {
        super( INSTALL_DEVICE_SOFTWARE,
               -1,
               macAddress,
               deviceURL,
               null,
               model,
               null );
    }
}
