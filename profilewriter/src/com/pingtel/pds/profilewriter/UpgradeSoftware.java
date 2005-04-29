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

public final class UpgradeSoftware extends ProfileWriterCommand {
    public UpgradeSoftware ( int profileType,
                             String macAddress,
                             String deviceURL,
                             String manufacturer,
                             String model,
                             String leafFileName ) {

        super( UPGRADE_SOFTWARE,
               profileType,
               macAddress,
               deviceURL,
               manufacturer,
               model,
               leafFileName);
    }
}
