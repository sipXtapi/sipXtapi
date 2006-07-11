/**
 * Copyright (c) 2001 Pingtel Corp.  All rights reserved.
 */

package org.sipfoundry.sipxphone.sys.appclassloader;

import java.net.URLConnection;
import java.net.URL;
import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayInputStream;

/**
 * ResourceCacheConnection
 *
 * @author Bob Andreasen
 */
public class ResourceCacheConnection extends URLConnection
{
//////////////////////////////////////////////////////////////////////////////
//  Constants
////

//////////////////////////////////////////////////////////////////////////////
//  Attributes
////
    byte[] m_data ;

//////////////////////////////////////////////////////////////////////////////
//  Construction
////
    public ResourceCacheConnection(URL url)
    {
        super(url) ;
    }

//////////////////////////////////////////////////////////////////////////////
//  Public Methods
////
    public void connect()
        throws IOException
    {
        String strKey = getURL().toExternalForm() ;

        m_data = ResourceCache.getResource(strKey) ;
        if (m_data == null)
        {
            throw new IOException("Resource not cached: " + strKey) ;
        }
    }
//////////////////////////////////////////////////////////////////////////////
//  Implementation (protected/private methods)
////

    public Object getContent()
        throws IOException
    {
        // Connect if not already connected
        if (!connected)
        {
            connect();
        }

        throw new IOException("not yet implemented!") ;

    }

    public InputStream getInputStream() throws IOException
    {
        // Connect if not already connected
        if (!connected)
        {
            connect();
        }

        return new ByteArrayInputStream(m_data) ;
    }

//////////////////////////////////////////////////////////////////////////////
//  Nested classes
////

}
