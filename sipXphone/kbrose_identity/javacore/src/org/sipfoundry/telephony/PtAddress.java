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

package org.sipfoundry.telephony ;

import org.sipfoundry.telephony.capabilities.* ;

import javax.telephony.* ;
import javax.telephony.capabilities.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.telephony.callcontrol.* ;


public class PtAddress extends PtWrappedObject implements Address
{
    /** cached copy of our name */
    String m_strName = null ;

    public PtAddress(long lHandle)
    {
        super(lHandle) ;

        // Prime the name when address is created...
        getName() ;
    }


    public String getName()
    {
        if (m_strName == null) {
            m_strName  = JNI_getName(m_lHandle) ;
/*
            // Clean off tag
            if (m_strName != null) {
                SipParser parser = new SipParser(m_strName) ;
                parser.setTag("") ;
                m_strName = parser.render() ;
            }
*/
        }

        return m_strName ;
    }

    /** not implemented */
    public Provider getProvider()
    {
        return null ;
    }

    public Terminal[] getTerminals()
    {
        Terminal terminals[] ;

        long lHandles[] = JNI_getTerminals(m_lHandle) ;
        if (lHandles != null) {
            terminals = new PtTerminal[lHandles.length] ;
            for (int i=0; i<lHandles.length; i++) {
                terminals[i] = new PtTerminal(lHandles[i]) ;
            }
        } else
            terminals = new PtTerminal[0] ;

        return terminals ;
    }

    /** not implemented */
    public Connection[] getConnections()
    {
        return null ;
    }

    /** not implemented */
    public void addObserver(AddressObserver observer)
            throws ResourceUnavailableException, MethodNotSupportedException
    {
        throw new MethodNotSupportedException() ;
    }

    /** not implemented */
    public AddressObserver[] getObservers()
    {
        return null ;
    }

    /** not implemented */
    public void removeObserver(AddressObserver observer)
    {
    }


    /** not implemented */
    public void addCallObserver(CallObserver observer)
            throws ResourceUnavailableException, MethodNotSupportedException
    {
        throw new MethodNotSupportedException() ;
    }


    /** not implemented */
    public CallObserver[] getCallObservers()
    {
        return null ;
    }


    /** not implemented */
    public void removeCallObserver(CallObserver observer)
    {
    }


    /** not implemented */
    public AddressCapabilities getCapabilities()
    {
        return new PtAddressCapabilities() ;
    }


    /** not implemented */
    public AddressCapabilities getAddressCapabilities(Terminal terminal)
        throws InvalidArgumentException, PlatformException
    {
        return new PtAddressCapabilities() ;
    }


    /** not implemented */
    public void addAddressListener(AddressListener listener)
            throws ResourceUnavailableException, MethodNotSupportedException
    {
        throw new MethodNotSupportedException() ;
    }


    /** not implemented */
    public AddressListener[] getAddressListeners()
    {
        return null ;
    }


    /** not implemented */
    public void removeAddressListener(AddressListener listener)
    {
    }


    /** not implemented */
    public void addCallListener(CallListener listener)
            throws ResourceUnavailableException, MethodNotSupportedException
    {
        throw new MethodNotSupportedException() ;
    }


    /** not implemented */
    public CallListener[] getCallListeners()
    {
        return null ;
    }


    /** not implemented */
    public void removeCallListener(CallListener listener)
    {
    }

    public String toString()
    {
        return getName() ;
    }


    /**
     * This is called when the VM decides that the object is no longer
     * useful (no more references are found to it) and it is time to
     * delete it.  At this point, we call down to the native world and
     * free the native object
     */
    protected void finalize()
        throws Throwable
    {
        JNI_finalize(m_lHandle) ;
        m_lHandle = 0 ;
    }

    /**
     * create an appropriate address object.  This method should determine if
     * an Address or a CallControlAddress should be returned
     */
    public static PtAddress fromNativeObject(long lHandle)
    {
        PtCallControlAddress address = null ;

        if (lHandle != 0) {
            address = new PtCallControlAddress(lHandle) ;
        }

        return address ;
    }



    /**
     * Determine if two Sip Addresses are equivalent given rules for address
     * comparisons.
     */
    public static boolean areSipAddressesEquivalent(String strAddress1, String strAddress2)
    {
        try
        {
            SipParser parser1 = new SipParser(strAddress1) ;
            SipParser parser2 = new SipParser(strAddress2) ;

            return (safeStringCompare(parser1.getUser(), parser2.getUser()) &&
                    safeStringCompare(parser1.getHost(), parser2.getHost()) &&
                    compareTags(parser1.getFieldParameterValue("tag"), parser2.getFieldParameterValue("tag")) &&
                    comparePorts(parser1.getPort(), parser2.getPort()) ) ;


        }
        catch (Exception e)
        {
            return safeStringCompare(strAddress1, strAddress2) ;
        }
    }


    /**
     * Safely determine if the two specified strings are identical.
     */
    private static boolean safeStringCompare(String strSource, String strTarget)
    {
        if ((strSource == null) && (strTarget != null))
            return false ;

        if ((strSource != null) && (strTarget == null))
            return false ;

        if ((strSource == null) && (strTarget == null))
            return true ;
        else
            return strSource.equalsIgnoreCase(strTarget) ;
    }

    /**
     * Determine if two tags match.  If one of the tags is null, then the tags
     * will match, otherwise, the tags will match if the tags match
     * identically.
     */
    private static boolean compareTags(String strSourceTag, String strTargetTag)
    {
        if ((strSourceTag == null) && (strTargetTag != null))
            return true ;

        if ((strSourceTag != null) && (strTargetTag == null))
            return true ;

        if ((strSourceTag == null) && (strTargetTag == null))
            return true ;
        else
            return strSourceTag.equalsIgnoreCase(strTargetTag) ;
    }


    /**
     * Compare ports with the assumption that port 0 is equivalent to port
     * 5060
     */
    private static boolean comparePorts(int iPort1, int iPort2)
    {
        if (iPort1 == 0)
            iPort1 = 5060 ;
        if (iPort2 == 0)
            iPort2 = 5060 ;

        return (iPort1 == iPort2) ;
    }



//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected static native String JNI_getName(long lHandle) ;
    protected static native long[] JNI_getTerminals(long lHandle) ;
    protected static native void   JNI_finalize(long lHandle) ;
}
