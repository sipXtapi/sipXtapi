/*
 * $Id: //depot/OPENDEV/sipXconfig/profilewriter/src/com/pingtel/pds/profilewriter/InstallSoftware.java#4 $
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

public class InstallSoftware extends ProfileWriterCommand {
    /**
     * 
     * @param profileType
     * @param macAddress
     * @param deviceURL
     * @param manufacturer
     * @param model
     * @param leafFileName
     */
    public InstallSoftware ( int profileType,
                             String macAddress,
                             String deviceURL,
                             String manufacturer,
                             String model,
                             String leafFileName ) {

        super( INSTALL_SOFTWARE,
               profileType,
               macAddress,
               deviceURL,
               manufacturer,
               model,
               leafFileName);
    }
}