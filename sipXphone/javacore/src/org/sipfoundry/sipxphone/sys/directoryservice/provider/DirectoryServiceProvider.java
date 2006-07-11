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

import javax.naming.directory.*;
import java.lang.*;
import java.util.*;

/**
 * This class is the base class for Directory Service Providers.
 *
 * This is the interface that is used by providers to add directory
 * services to the system.
 * This uses a singleton pattern.
 */
public class DirectoryServiceProvider
{
    
    protected DirectoryServiceProvider()
    {
    }
        
    /**
     * This method clears the directory, removing every entry in the directory.
     */
    public void clear()
    {
    }
    
    public void addEntry(String name, BasicAttributes attr)
    {
    }
    
    public void deleteEntry(String name) throws IllegalArgumentException
    {
    }
    
    public BasicAttributes lookupName(String name) throws IllegalArgumentException
    {
        return null;
    }

    /**
     * Look up an entry in the phone book given the caller ID.
     */
    public BasicAttributes lookupCallerID(String callerID) throws IllegalArgumentException
    {
        return null;
    }
    
    /**
     * This method returns a list of all of the entries in the directory.
     */
    public Vector getEntries()
    {
        return null;
    }
}
