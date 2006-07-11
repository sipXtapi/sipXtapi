
/*
 * $Id: $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package org.sipfoundry.sipxphone.sys.appclassloader;

import java.io.IOException;
import java.net.URL;

/**
 * Copyright (c) 2001 Pingtel Corp.  All rights reserved.
 * Manage classes and resources from code in a URL.
 *
 */
public class AppClassLoader extends URLClassLoader 
{
    /**
     * @param codebase URL to jar
     */
    public AppClassLoader(URL codebase) 
    {
        super(codebase);
        try 
        {
            readAll();
        } 
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }    
}

