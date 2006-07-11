package org.sipfoundry.sipxphone.sys.appclassloader;

import java.lang.*;
import java.net.*;
import java.io.*;
import java.util.*;
import java.util.zip.*;

/**
 * Copyright (c) 2001 Pingtel Corp.  All rights reserved.
 * Load classes and resources from a jar file described by a URL
 */
public class URLClassLoader extends ClassLoader 
{
    /** original url to jar file */
    private URL m_url;

    /** list of raw un-realized class byte streams */
    private Hashtable m_rawClasses;

    /** cache of classes */
    private Hashtable m_classes;

    public URLClassLoader(URL url)
    {
        m_url = url;
        m_classes = new Hashtable(128) ;
        m_rawClasses = new Hashtable(128) ;
    }

    /**
     * stub implementation
     */
    protected Class loadClass(String className, boolean resolve)
        throws ClassNotFoundException
    {
        Class c = (Class)m_classes.get(className);
        if (c != null)
        {
            if (resolve)
            {
                resolveClass(c);
            }
        }
        else
        {
            byte classData[] = (byte[]) m_rawClasses.get(className) ;
            if (classData != null)
            {
                m_rawClasses.remove(className) ;
                c = defineClass(classData, 0, classData.length);
                m_classes.put(c.getName(), c);
                if (resolve)
                {
                    resolveClass(c) ;
                }
            }
            else
            {
                // try parent's(system's) class loader
                c = this.getClass().forName(className);
            }
        }

        return c;
    }

    /**
     * Return stream to resources like property files, audio clips, etc.
     *
     * @return null is resource is not found
     */
    public InputStream getResourceAsStream(String name)
    {
        InputStream resource = null;
        URL urlKey = ResourceCache.buildResourceKey(m_url, name);
        if (urlKey != null)
        {
            try
            {
                resource = urlKey.openStream();
            }
            catch (IOException io)
            {
                // return null stream is ok
            }
        }
        else
        {
            resource = getSystemResourceAsStream(name);
        }

        return resource;
    }

    /**
     * Generate a URL that will lead URL protocol handler back to this 
     * instance to load this resource.
     */
    public URL getResource(String name)
    {
        return ResourceCache.buildResourceKey(m_url, name);
    }

    /**
     * Load all classes and resources
     */
    protected void readAll()
        throws ClassNotFoundException
    {    
        byte[] classData = new byte[1024*100] ;

        InputStream classStream = null;
        try
        {
            classStream = m_url.openStream();
            ZipInputStream zip = new ZipInputStream(classStream);
            ZipEntry entry = zip.getNextEntry();
            while (entry != null)
            {
                if (!entry.isDirectory())
                {
                    int totalRead = 0 ;
                    int bytesRead = 0 ;
                    boolean error = false ;
                    do
                    {
                        int bytesToRead = classData.length - totalRead ;
                        if (bytesToRead == 0)
                        {
                            System.out.println("Skipping entry " + entry.getName() + ": over " + 
                                    classData.length + " bytes.") ; 
                            error = true ;
                        }
                        bytesRead = zip.read(classData, totalRead, classData.length - totalRead) ;
                        if (bytesRead > 0)
                        {
                            totalRead += bytesRead ;
                        }
                    } while (bytesRead > 0);

                    if (!error)
                    {
                        // java class 
                        if (entry.getName().endsWith(".class"))
                        {
                            byte[] classDataCopy = new byte[totalRead];
                            System.arraycopy(classData, 0, classDataCopy, 0, totalRead);
                            String strName = entry.getName().replace('/', '.') ;
                            int index = strName.lastIndexOf(".class") ;
                            strName = strName.substring(0, index) ;
                            m_rawClasses.put(strName, classDataCopy) ;
                        }
                        else
                        {
                            byte[] resource = new byte[totalRead];
                            System.arraycopy(classData, 0, resource, 0, totalRead);
                            URL keyUrl = ResourceCache.buildResourceKey(m_url, entry.getName());
                            ResourceCache.addResource(keyUrl.toExternalForm(), resource);
                        }
                    }
                }

                zip.closeEntry();
                entry = zip.getNextEntry();
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
        finally
        {
            if (classStream != null)
            {
                try
                {
                    classStream.close();
                } 
                catch (IOException ignore)
                {
                }
            }
        }
    }
}
