/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/stapi/PCallData.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.stapi ;

import java.util.* ;

class PCallData
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    static private PCallData m_instance = null ;
    static private Hashtable m_htData ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    private PCallData()
    {
        m_htData = new Hashtable() ;
    }


    protected static PCallData getInstance()
    {
        if (m_instance == null)
        {
            m_instance = new PCallData() ;
        }
        return m_instance ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get a piece of call/application data for the specified call-id and key
     */
    public synchronized Object getData(String strCallId, Object objKey)
    {
        Object objRC = null ;

        Hashtable htCallIdData = (Hashtable) m_htData.get(strCallId) ;
        if (htCallIdData != null)
        {
            objRC = htCallIdData.get(objKey) ;
        }
        return objRC ;
    }


    /**
     * Set a piece of call/application data for the specified call-id
     */
    public synchronized Object setData(String strCallId, Object objKey, Object objData)
    {
        Object objRC = null ;

        Hashtable htCallIdData = (Hashtable) m_htData.get(strCallId) ;
        if (htCallIdData == null)
        {
            htCallIdData = new Hashtable() ;
        }

        objRC = htCallIdData.put(objKey, objData) ;
        m_htData.put(strCallId, htCallIdData) ;

        return objRC ;
    }


    /**
     * Removes a piece of call/application data for the specified a call-id
     */
    public synchronized Object removeData(String strCallId, Object objKey)
    {
        Object objRC = null ;

        Hashtable htCallIdData = (Hashtable) m_htData.get(strCallId) ;
        if (htCallIdData == null)
        {
            htCallIdData = new Hashtable() ;
        }

        objRC = htCallIdData.remove(objKey) ;
        m_htData.put(strCallId, htCallIdData) ;

        return objRC ;
    }


    /**
     * Dump all of the piece of application specific data to the specified
     * output stream.
     */
    public synchronized void dump(java.io.PrintStream out)
    {
        Enumeration enumKeys =  m_htData.keys() ;
        while ((enumKeys != null) && enumKeys.hasMoreElements())
        {
            String strCallId = (String) enumKeys.nextElement() ;
            out.println("Dumping application data for: " + strCallId) ;

            Hashtable htData = (Hashtable) m_htData.get(strCallId) ;
            if (htData != null)
            {
                Enumeration enumData = htData.keys() ;
                while ((enumData != null) && enumData.hasMoreElements())
                {
                    Object objKey = enumData.nextElement() ;
                    Object objData = htData.get(objKey) ;

                    out.println("  " + objKey + " : " + objData) ;
                }
            }
        }
    }


    /**
     * Clear all of the application data for the specified call id.
     */
    public synchronized void removeAllData(String strCallId)
    {
        m_htData.remove(strCallId) ;
    }
}
