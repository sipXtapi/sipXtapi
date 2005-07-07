/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/stapi/ConnectionStateCache.java#2 $
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
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.util.* ;

/**
 * <<DOC_ME>>
 *
 * @author Robert J. Andreasen
 */
class ConnectionStateCache
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final boolean DEBUG = false ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    Hashtable m_htStates ;      // map of connection states
    Hashtable m_htAddresses ;   // map of original (uncleaned) addresses
    Vector    m_vHeldRemotes ;  // Hint of remote connections that we have held

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default no argument constructor
     */
    public ConnectionStateCache()
    {
        m_htStates = new Hashtable() ;
        m_htAddresses = new Hashtable() ;
        m_vHeldRemotes = new Vector() ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public synchronized void setState(PAddress address, int iState)
    {
        String strCleanAddress ;

        // Clear any non-tagged version of this address
        strCleanAddress = cleanAddress(address, false) ;
        m_htStates.remove(strCleanAddress) ;
        m_htAddresses.remove(strCleanAddress) ;


        // Add the state
        strCleanAddress = cleanAddress(address, true) ;
        m_htStates.put(strCleanAddress, new Integer(iState)) ;
        m_htAddresses.put(strCleanAddress,address) ;

        debugln("Set state to " + iState + " for " + strCleanAddress) ;
    }


    public synchronized boolean containsAddress(PAddress address)
    {
        boolean bContainsAddress = false ;
        String strCleanAddress = cleanAddress(address, true) ;

        bContainsAddress = m_htStates.contains(strCleanAddress) ;

        debugln("Contains " + strCleanAddress + ": " + bContainsAddress) ;

        return bContainsAddress ;
    }


    public synchronized int getState(PAddress address)
    {
        int iState = PCall.CONN_STATE_UNKNOWN ;

        String strCleanAddress = cleanAddress(address, true) ;
        Integer intValue = (Integer) m_htStates.get(strCleanAddress) ;
        if (intValue != null)
        {
            iState = intValue.intValue() ;
        }

        debugln("State for " + strCleanAddress + ": " + iState) ;

        return iState ;
    }


    public synchronized void clearState(PAddress address)
    {
        String strCleanAddress = cleanAddress(address, true) ;

        m_htStates.remove(strCleanAddress) ;
        m_htAddresses.remove(strCleanAddress) ;

        debugln("Cleared state for " + strCleanAddress) ;
    }


    public synchronized void clear()
    {
        m_htStates.clear() ;
        m_htAddresses.clear() ;
        m_vHeldRemotes.removeAllElements();
        debugln("Cleared states") ;
    }


    /**
     * returns an enumeration of addresses
     */
    public synchronized Enumeration getAddresses()
    {
        Hashtable htClone = (Hashtable) m_htAddresses.clone() ;
        Enumeration enum = htClone.elements() ;

        return enum ;
    }


    /**
     * Note that an address was placed on hold
     */
    public synchronized void setHeldRemote(PAddress address, boolean bHeld)
    {
        String strAddress = cleanAddress(address, true) ;

        if (bHeld)
        {
            // Set Held State
            if (!m_vHeldRemotes.contains(strAddress))
            {
                m_vHeldRemotes.addElement(strAddress) ;
            }
        }
        else
        {
            // Remove Held State
            if (m_vHeldRemotes.contains(strAddress))
            {
                m_vHeldRemotes.removeElement(strAddress) ;
            }
        }

        debugln("Setting remote held state for " + strAddress + ": " + bHeld) ;
    }


    /**
     * Determine if an address was placed on hold?
     */
    public synchronized boolean isRemoteHeld(PAddress address)
    {
        String strAddress = cleanAddress(address, true) ;

        return m_vHeldRemotes.contains(strAddress) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /**
     * Cleans an address and returns it as a string
     */
    protected String cleanAddress(PAddress address, boolean bKeepTag)
    {
        String strAddress = address.toString() ;

        // Add the state
        if (SipParser.isSIPURL(strAddress))
        {
            try
            {
                SipParser parser = new SipParser(strAddress) ;

                parser.removeAllHeaderParameters() ;
                parser.removeAllURLParameters() ;
                parser.setDisplayName("") ;
                parser.setIncludeAngleBrackets(true);

                if (!bKeepTag)
                    parser.removeAllFieldParameters() ;

                strAddress = parser.render() ;
            }
            catch (Exception e)
            {
                SysLog.log(e) ;
            }
        }

        debugln("Cleaned Address: " + address.toString() + " -> " + strAddress) ;

        return strAddress ;
    }


    protected void debugln(String strInfo)
    {
        if (DEBUG)
        {
            System.out.println("ConnectionStateCache: " + strInfo) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
