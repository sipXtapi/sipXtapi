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


package com.pingtel.pds.pgs.common;

import java.io.InputStream;
import java.io.IOException;

import com.pingtel.pds.common.VersionInfo;
import com.pingtel.pds.sipxfacade.SipxConfigFacadeFactory;

/**
 * PGSVersionInfo returns the version and build details for the PGS.   These
 * values are added to the PGS as part of the build process.
 */
public class PGSVersionInfo {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////
    public PGSVersionInfo() {}

    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    /**
     * getVersion returns a 'stringified' representation of the version/build
     * information for the PGS
     *
     * @return version/build information for PGS
     */
    public String getVersion () {
        InputStream versionStream =
                getClass().getClassLoader().getResourceAsStream( "buildVersion.properties" );

        try {
            VersionInfo versionInfo = new VersionInfo ( versionStream );
            String facade = SipxConfigFacadeFactory.getFacade().getClass().getName();
            return versionInfo.getVersion() + "-" + facade;
        }
        catch ( IOException ex ) {
            throw new RuntimeException ( ex.toString() );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////

    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////

    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}
