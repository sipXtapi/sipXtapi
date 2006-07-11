/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys.directoryservice.provider;

/**
 * This class is used to register the directory service providers
 * in the system.
 */
 
public class DirectoryServiceProviderRegistry
{
    // Enumerate the names of the directory providers that are registered.
    // This is used by the UI to present the choices to the users.
    
    // Given the name of a provider, get a reference to the singleton.
    
    // Add an entry to the table
    
    // Remove an entry from the table
    
    public static DirectoryServiceProvider getDirectoryServiceProvider(String directoryName) throws IllegalArgumentException
    {
        if (directoryName.compareTo("speed_dial") == 0)
        {
            return SpeedDialDSP.getInstance();
        } else {
            throw new IllegalArgumentException("Directory Service Provider not found");
        }
    }
}
