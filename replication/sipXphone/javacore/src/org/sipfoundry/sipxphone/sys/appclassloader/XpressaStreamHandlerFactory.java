/**
 * Copyright (c) 2001 Pingtel Corp.  All rights reserved.
 */

package org.sipfoundry.sipxphone.sys.appclassloader;

import java.net.URLStreamHandlerFactory;
import java.net.URLStreamHandler;

/**
 * XpressaStreamHandlerFactory
 *
 * @author Bob Andreasen
 */
public class XpressaStreamHandlerFactory implements URLStreamHandlerFactory
{
//////////////////////////////////////////////////////////////////////////////
//  Constants
////

//////////////////////////////////////////////////////////////////////////////
//  Attributes
////

//////////////////////////////////////////////////////////////////////////////
//  Construction
////
    public XpressaStreamHandlerFactory()
    {

    }

//////////////////////////////////////////////////////////////////////////////
//  Public Methods
////
    public URLStreamHandler createURLStreamHandler(String scheme)
    {
        URLStreamHandler handler = null ;

        if (scheme.equalsIgnoreCase("http"))
        {
            handler =  new sun.net.www.protocol.http.Handler();
        }
        else if (scheme.equalsIgnoreCase("ftp"))
        {
            handler = new sun.net.www.protocol.ftp.Handler();
        }
        else if (scheme.equalsIgnoreCase("file"))
        {
            handler = new sun.net.www.protocol.file.Handler();
        }
        else if (scheme.equalsIgnoreCase("systemresource"))
        {
            handler = new sun.net.www.protocol.systemresource.Handler();
        }
        else if (scheme.equalsIgnoreCase(ResourceCache.RESOURCE_SCHEME))
        {
            handler = new ResourceCacheHandler();
        }

        return handler ;
    }


//////////////////////////////////////////////////////////////////////////////
//  Implementation (protected/private methods)
////

//////////////////////////////////////////////////////////////////////////////
//  Nested classes
////
}

