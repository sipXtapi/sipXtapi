/**
 * Copyright (c) 2001 Pingtel Corp.  All rights reserved.
 */

package org.sipfoundry.sipxphone.sys.appclassloader;

import java.util.Hashtable;
import java.util.Enumeration;
import java.net.URL;

/**
 * ResourceCache
 */
public class ResourceCache
{
//////////////////////////////////////////////////////////////////////////////
//  Constants
////
    protected static final String RESOURCE_SEPARATOR_IN_URL = "#";
    protected static final String RESOURCE_SCHEME = "resourceCache";

//////////////////////////////////////////////////////////////////////////////
//  Attributes
////
    static private Hashtable s_hashResources = new Hashtable();

//////////////////////////////////////////////////////////////////////////////
//  Construction
////
    /**
     * Class is never created -- use static methods
     */
    private ResourceCache()
    {
    }

//////////////////////////////////////////////////////////////////////////////
//  Public Methods
////
    /**
     * Dump the allocated resource list
     */
    static public void dumpResources()
    {
        Enumeration enumKeys ;

        enumKeys = s_hashResources.keys() ;
        while (enumKeys.hasMoreElements())
        {
            Object objKey = enumKeys.nextElement() ;
            if (objKey != null)
            {
                System.out.println("ResourceCache: dumped: " + objKey.toString()) ;
            }
        }
    }

   /**
     * Purge all of the allocated resources with the specified key, where
     * the key is the class/package name
     */
    static public void purgeResources(String strPrefix)
    {
        int iPurgedResources = 0 ;

        Enumeration enumKeys ;
        enumKeys = s_hashResources.keys() ;
        while (enumKeys.hasMoreElements())
        {
            Object objKey = enumKeys.nextElement() ;
            if (objKey != null)
            {
                String strKey = objKey.toString() ;
                if (strKey.startsWith(strPrefix))
                {
                    s_hashResources.remove(objKey) ;
                    iPurgedResources++ ;
                }
            }
        }
    }

    /**
     * Add a resource to the cache
     *
     * @param strKey unique name of the resource -- this should be the
     *        "toExternalForm()" of buildResourceKey.
     * @param content Raw bits of resource data
     */
    static public void addResource(String strKey, byte[] content)
    {
        s_hashResources.put(strKey, content) ;
    }


    /**
     * Get a resource given a unique key.
     *
     * @param strKey unique name of the resource -- this should be the
     *        "toExternalForm()" of buildResourceKey.
     * @return the bit stream of null if not found
     */
    static public byte[] getResource(String strKey)
    {
        byte[] data = (byte[]) s_hashResources.get(strKey) ;

        return data ;
    }


    /**
     * Build a unique URL key given a jar and resource name
     */
    static public URL buildResourceKey(URL urlJar, String strResourceName)
    {
        URL urlResource = null ;

        try
        {
            urlResource = new URL(RESOURCE_SCHEME,
                    urlJar.getHost(),
                    urlJar.getPort(),
                    urlJar.getFile() + RESOURCE_SEPARATOR_IN_URL + strResourceName);
        }
        catch (Exception e)
        {
            e.printStackTrace() ;
        }

        return urlResource ;
    }

//////////////////////////////////////////////////////////////////////////////
//  Implementation (protected/private methods)
////

//////////////////////////////////////////////////////////////////////////////
//  Nested classes
////

}
