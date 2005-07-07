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

import javax.telephony.* ;
import javax.telephony.capabilities.* ;

import org.sipfoundry.telephony.capabilities.* ;
import org.sipfoundry.telephony.callcontrol.capabilities.* ;

import org.sipfoundry.sipxphone.sys.util.PingerInfo ;

public class PtProvider extends PtWrappedObject implements Provider
{
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Please use PtProvider.getProvider() to obtain a reference to a provider.
     */
    public PtProvider(long lHandle)
    {
        super(lHandle) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public int getState()
    {
        return  0 ;
    }

    public String getName()
    {
        return this.getClass().getName() ;
    }

    /** not implemented */
    public Call[] getCalls() throws ResourceUnavailableException
    {
        return null ;
    }


    public Address getAddress(String strAddress) throws InvalidArgumentException
    {
        return new PtAddress(JNI_getAddress(m_lHandle, strAddress)) ;
    }

    public Address[] getAddresses() throws ResourceUnavailableException
    {
         PtAddress addresses[] = null ;

        long lAddresses[] = JNI_getAddresses(m_lHandle) ;
        addresses = new PtAddress[lAddresses.length] ;
        for (int i=0;i<lAddresses.length; i++) {
            addresses[i] = new PtAddress(lAddresses[i]) ;
        }
        return addresses ;
    }


    public Terminal[] getTerminals() throws ResourceUnavailableException
    {
        PtTerminal terminals[] = null ;

        long lTerminals[] = JNI_getTerminals(m_lHandle) ;
        terminals = new PtTerminal[lTerminals.length] ;
        for (int i=0;i<lTerminals.length; i++) {
            terminals[i] = new PtTerminal(lTerminals[i]) ;
        }
        return terminals ;
    }

    public Terminal getTerminal(String name) throws InvalidArgumentException
    {
        return new PtTerminal(JNI_getTerminal(m_lHandle, name)) ;
    }

    /** not implemented */
    public void shutdown()
    {

    }


    public Call createCall() throws ResourceUnavailableException, InvalidStateException, PrivilegeViolationException, MethodNotSupportedException
    {
        PingerInfo pingerInfo = PingerInfo.getInstance() ;
        Call call = null ;

        if (pingerInfo.getSimultaneousConnections() >= pingerInfo.getMaxSimultaneousConnections()) {
            System.out.println("MAX CONNECTIONS REACHED") ;
            System.out.println("Simultaneous Connections: " + pingerInfo.getSimultaneousConnections()) ;
            System.out.println("         Max Connections: " + pingerInfo.getMaxSimultaneousConnections()) ;

            throw new ResourceUnavailableException(ResourceUnavailableException.UNSPECIFIED_LIMIT_EXCEEDED, "max simultaneous connections exceeded") ;
        } else {
            long lCallHandle = JNI_createCall(m_lHandle) ;
            if (lCallHandle != 0) {
                call = PtCall.fromNativeObject(lCallHandle) ;
            }
        }
        return call ;
    }

    /** not implemented */
    public void addObserver(ProviderObserver observer) throws ResourceUnavailableException, MethodNotSupportedException
    {
        throw new MethodNotSupportedException() ;
    }

    /** not implemented */
    public ProviderObserver[] getObservers()
    {
        return null ;
    }

    /** not implemented */
    public void removeObserver(ProviderObserver observer)
    {

    }

    /** not implemented */
    public ProviderCapabilities getProviderCapabilities()
    {
        return new PtProviderCapabilities() ;
    }

    /** not implemented */
    public CallCapabilities getCallCapabilities()
    {
        return new PtCallControlCallCapabilities() ;
    }

    /** not implemented */
    public AddressCapabilities getAddressCapabilities()
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
    public TerminalConnectionCapabilities getTerminalConnectionCapabilities(Terminal terminal)
        throws InvalidArgumentException, PlatformException
    {
        return new PtTerminalConnectionCapabilities() ;
    }

    /** not implemented */
    public TerminalCapabilities getTerminalCapabilities()
    {
        return new PtTerminalCapabilities() ;
    }

    /** not implemented */
    public ConnectionCapabilities getConnectionCapabilities()
    {
        return new PtCallControlConnectionCapabilities() ;
    }

    /** not implemented */
    public TerminalConnectionCapabilities getTerminalConnectionCapabilities()
    {
        return new PtCallControlTerminalConnectionCapabilities() ;
    }

    /** not implemented */
    public ProviderCapabilities getCapabilities()
    {
        return new PtProviderCapabilities() ;
    }

    /** not implemented */
    public ProviderCapabilities getProviderCapabilities(Terminal terminal) throws InvalidArgumentException, PlatformException
    {
        return new PtProviderCapabilities() ;
    }

    /** not implemented */
    public CallCapabilities getCallCapabilities(Terminal terminal, Address address)
        throws InvalidArgumentException, PlatformException
    {
        return new PtCallControlCallCapabilities() ;
    }

    /** not implemented */
    public ConnectionCapabilities getConnectionCapabilities(Terminal terminal, Address address)
        throws InvalidArgumentException, PlatformException

    {
        return new PtCallControlConnectionCapabilities() ;
    }


    /** not implemented */
    public TerminalCapabilities getTerminalCapabilities(Terminal terminal) throws InvalidArgumentException, PlatformException
    {
        return new PtTerminalCapabilities() ;
    }

    /** not implemented */
    public void addProviderListener(ProviderListener listener)
        throws ResourceUnavailableException, MethodNotSupportedException
    {
        throw new MethodNotSupportedException() ;
    }

    /** not implemented */
    public ProviderListener[] getProviderListeners()
    {
        return null ;
    }

    /** not implemented */
    public void removeProviderListener(ProviderListener listener)
    {

    }


//////////////////////////////////////////////////////////////////////////////
// Package protected members
////
    /**
     * Get a reference (singleton?) to a PTAPI provider.
     */
    protected static PtProvider getProvider(String strUser, String strPasswd, String strServer)
    {
        return new PtProvider(JNI_getProvider(strUser, strPasswd, strServer)) ;
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



//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected static native long   JNI_new_PtProvider() ;
    protected static native void   JNI_delete_PtProvider(long lhandle) ;
    protected static native long   JNI_getProvider(String strUser, String strPassword, String strServer) ;
    protected static native long   JNI_getTerminal(long lHandle, String strName) ;
    protected static native long[] JNI_getTerminals(long lHandle) ;
    protected static native long[] JNI_getAddresses(long lHandle) ;
    protected static native long   JNI_createCall(long lHandle) ;
    protected static native long   JNI_getAddress(long lHandle, String strAddress) ;
    protected static native void   JNI_finalize(long lHandle) ;
}
