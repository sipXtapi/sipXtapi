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


package org.sipfoundry.sipxphone.sys ;

import java.io.*;
import java.util.Enumeration;
import java.util.Hashtable;
import java.lang.Thread;

import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.awt.event.PActionEvent ;
import org.sipfoundry.sipxphone.awt.event.PActionListener ;

import org.sipfoundry.sipxphone.sys.startup.PingerApp ;
import org.sipfoundry.util.SysLog;
/**
 * The repository is a persistent cache for the Properties.
 * It is saved and restored from the local file system.
 */
public class Repository
        implements PActionListener
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    protected static String TURD = "RepositoryVersionTurd" ;
    protected static String VERSION_TURD = "2.0.0.1" ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Hashtable  m_cache ;            // The actual cache
    private String m_cacheFileName ;        // File spec of the cache
    private String m_oldCacheVersion ;      // Old version if upgrade
    private String m_newCacheVersion ;      // New/Current cache version
    private int    m_iFlushProcrastinated ; // Is the cache dirty?

    private static Repository m_reference ; // Reference to the Repository



//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Private Constructor; to guard against rogue instantiation
     */
    protected Repository()
    {
        FileInputStream fis ;

        // Figure out file name
        m_cacheFileName = PingerInfo.getInstance().getFlashFileSystemLocation() + File.separator +  "cache.ser";

        // deserialize the repository from flash
        try
        {
            fis = new FileInputStream(m_cacheFileName);
            ObjectInputStream ois = new ObjectInputStream(fis);

            m_cache = (Hashtable) ois.readObject();
            fis.close();

            m_oldCacheVersion = (String) m_cache.get((Object) TURD);

            if ((m_oldCacheVersion != null) && (m_oldCacheVersion.compareTo(VERSION_TURD) != 0))
            {
                System.out.println("Repository: Upgrading to: " + VERSION_TURD + " from: " + m_oldCacheVersion) ;

                m_cache.put((Object) TURD, VERSION_TURD);
                m_newCacheVersion = VERSION_TURD;
            }
        }
        catch (Exception e)
        {
            m_cache = new Hashtable();
            m_cache.put((Object) TURD, VERSION_TURD) ;
        }

        m_iFlushProcrastinated = 0 ;
    }


    /**
     * Method used to get reference to the singleton for this class
     */
    public static Repository getInstance()
    {
        if (m_reference == null)
            m_reference = new Repository() ;


        return m_reference ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * This method is used to flush the changes to the repository to flash.
     */
    public synchronized void flush()
    {
        FileOutputStream fos ;

        // Serialize the repository
        try
        {
            System.out.print("Flushing Repository...") ;

            fos = new FileOutputStream(m_cacheFileName);
            ObjectOutputStream oos = new ObjectOutputStream(fos);

            oos.writeObject(m_cache);

            // Make sure it is actually written...
            oos.flush() ;
            oos.reset() ;
            fos.close() ;

            m_iFlushProcrastinated = 0 ;

            System.out.println("Done") ;
        }
        catch (Exception e)
        {
            SysLog.log(e);
            Shell.getInstance().showUnhandledException(e, false) ;
        }
    }


    /**
     * This method is used to get what the old version of the repository
     * was before the system was started this time.
     */
    public String getOldRepositoryVersion()
    {
        return m_oldCacheVersion;
    }


    /**
     * This method is used to get what the new version of the repository
     * is now. This is the current version of the software that is running.
     */
    public String getNewRepositoryVersion()
    {
        return m_newCacheVersion;
    }


    /**
     * Return an Enumeration that can be used to go through
     * the list of all of the objects in the repository.
     *
     * @return An Enumeration of all of the elements in the repository.
     */
    public Enumeration elements()
    {
        return m_cache.elements();
    }


    /**
     * Search the repository for the key specified.
     *
     * @return Returns true if the key is contained in the repository
     */

    public boolean containsKey(String key)
    {
        return m_cache.containsKey((Object) key);
    }


    /**
     * This method is used to retrieve an object based on the search key
     *
     * @param key This is the string to use to retrieve the object
     */
    public Object get(String key)
        throws IOException
    {
        // First check to see if the config server is up
        // if it is, then update the object in the local cache based on the config server
        // if the server is not up, then return the object from the local cache

        // Somewhere in here, it would be nice to know if we are in a power failure
        // situation. Maybe the way to do this is to have a transaction with the
        // config server that asks it whether it is swamped, or not.


        Object objRC = null ;
        try {
            RepositoryDataWrapper wrapper = (RepositoryDataWrapper) m_cache.get(key) ;
            if (wrapper != null) {
                objRC = wrapper.getObject() ;
            }
        } catch (ClassCastException cce) {
            System.out.println("Repository: Unexpected data in repository under key: " + key) ;
            remove(key) ;
        } catch (ClassNotFoundException cnfe) {
            System.out.println("Repository: Unknown data in repository under key: " + key) ;
            remove(key) ;
        } catch (OptionalDataException ode) {
            System.out.println("Repository: Invalid format in repository under key: " + key) ;
            remove(key) ;
        }

        return objRC ;
    }


    /**
     * This method is used to store and associate a key with the object.
     * This same method can be used to overwrite an object with the same
     * name.
     *
     * @param key This is the string to store with the object
     * @param outputObjectStream This is the object to store.
     */

    public void store(String key, Object object)
        throws IOException
    {
        // store the object in the local cache
        m_cache.put((Object) key, new RepositoryDataWrapper((Serializable) object));
        markDirty() ;
    }


    /**
     * Delete the object with this key
     */
    public Object remove(String key)
    {
        Object objRC =  m_cache.remove((Object) key) ;
        if (objRC != null)
            markDirty() ;

        return objRC ;
    }


    /**
     * This method is used for debugging purposes.
     *
     * @return The list of keys in the repository
     */
    public String toString()
    {
        String retval = "Repository keys:\n";

        Enumeration keys = m_cache.keys();

        while (keys.hasMoreElements()) {
            retval += keys.nextElement() + "\n";
        }

        return retval;
    }


    /**
     * PActionListener handler invoked when it is time to flush the repository.
     */
    public void actionEvent(PActionEvent event)
    {
        if(event.getActionCommand().equals(Timer.ACTION_TIMER_FIRED))
        {
            flush() ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Notes that the repository is dirty and will need to be flushed at some
     * point.
     */
    protected synchronized void markDirty()
    {
        m_iFlushProcrastinated++ ;



        if (m_iFlushProcrastinated > Settings.getInt("REPOSITORY_MAX_FLUSH_PROCRASTINATION", 30))
        {
            Timer.getInstance().removeTimers(this) ;
            flush() ;
        }
        else
        {
            Timer.getInstance().resetTimer(Settings.getInt("REPOSITORY_FLUSH_PROCRASTINATION_IN_SECS", 30)  * 1000, this, null) ;
        }
    }
}
