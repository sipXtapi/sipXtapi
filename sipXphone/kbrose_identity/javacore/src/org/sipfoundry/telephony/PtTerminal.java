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
import javax.telephony.phone.* ;
import javax.telephony.phone.capabilities.* ;

import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.telephony.phone.* ;
import org.sipfoundry.telephony.capabilities.* ;
import org.sipfoundry.telephony.phone.capabilities.* ;

public class PtTerminal extends PtWrappedObject implements PhoneTerminal
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** cached copy of our component groups */
    private ComponentGroup  m_componentGroups[] = null ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Object is not designed to be instantiated.  Please use <i>getTerminal()</i>
     * from the provider to gain a reference to a PtTerminal.
     */
    public PtTerminal(long lHandle)
    {
        super(lHandle) ;
    }



//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /* JTAPI EXTENSION */
    public void setCodecCPULimit(int iLimit)
    {
        JNI_setCodecCPULimit(m_lHandle, iLimit) ;
    }


    public PtComponent[] getComponents()
    {
        PtComponent components[] ;

        long lHandles[] = JNI_getComponents(m_lHandle) ;

        components = new PtComponent[lHandles.length] ;
        for (int i=0;i<lHandles.length; i++) {
            components[i] = PtComponent.createComponent(lHandles[i]) ;
        }

        return components ;
    }


    public String getName()
    {
        return JNI_getName(m_lHandle) ;
    }


    public ComponentGroup[] getComponentGroups()
    {
        if (m_componentGroups == null) {
            long lGroups[] = JNI_getComponentGroups(m_lHandle) ;
            m_componentGroups = new PtComponentGroup[lGroups.length] ;
            for (int i=0;i<lGroups.length; i++) {
                m_componentGroups[i] = new PtComponentGroup(lGroups[i]) ;
            }
        }

        return m_componentGroups ;
    }


    public Provider getProvider()
    {
        return null ;
    }


    public Address[] getAddresses()
    {
        long lAddresses[] = JNI_getAddresses(m_lHandle) ;

        PtAddress addresses[] = new PtAddress[lAddresses.length] ;
        for (int i=0; i<addresses.length; i++) {
            addresses[i] = PtAddress.fromNativeObject(lAddresses[i]) ;
        }

        return addresses ;
    }


    public TerminalConnection[] getTerminalConnections()
    {
        return null ;
    }


    /**
     * TerminalObserver is current unsupported
     */
    public void addObserver(TerminalObserver observer)
        throws ResourceUnavailableException, MethodNotSupportedException
    {
        throw new MethodNotSupportedException() ;
    }
    public TerminalObserver[] getObservers() { return null ; }
    public void removeObserver(TerminalObserver observer) { }


    /**
     * CallObserver is current unsupported
     */
    public void addCallObserver(CallObserver observer)
        throws ResourceUnavailableException, MethodNotSupportedException
    {
        throw new MethodNotSupportedException() ;
    }
    public CallObserver[] getCallObservers() { return null ; }
    public void removeCallObserver(CallObserver observer) { }


    /**
     * Add a Terminal listener
     */
    public void addTerminalListener(TerminalListener listener)
        throws ResourceUnavailableException, MethodNotSupportedException
    {
        long lFilter = PtTerminalComponentAdapter.IGNORE_NONE ;
        if (listener instanceof PtTerminalComponentAdapter) {
            lFilter = ((PtTerminalComponentAdapter) listener).getEventFilter() ;
        }

        JNI_addTerminalListener(m_lHandle, listener, lFilter) ;
    }


    /**
     * Remove a Terminal Listener
     */
    public void removeTerminalListener(TerminalListener listener)
    {
        JNI_removeTerminalListener(m_lHandle, listener) ;
    }


    /**
     * currently not supported - give me the list of Terminal Listeners
     */
    public TerminalListener[] getTerminalListeners()
    {
        return null ;
    }


    /**
     * Add a Call Listener
     */
    public void addCallListener(CallListener listener)
        throws ResourceUnavailableException, MethodNotSupportedException
    {
        int iHashCode = listener.hashCode() ;

        if (listener instanceof TerminalConnectionListener) {
            JNI_addCallListener_term(m_lHandle, (TerminalConnectionListener) listener, iHashCode, 0) ;
        } else if (listener instanceof ConnectionListener) {
            JNI_addCallListener_conn(m_lHandle, (ConnectionListener) listener, iHashCode, 0) ;
        } else {
            JNI_addCallListener_call(m_lHandle, listener, iHashCode, 0) ;
        }
    }


    /**
     * EXTENSION
     */
    public void addCallListener(CallListener listener, long ulExcludeFilter)
        throws ResourceUnavailableException, MethodNotSupportedException
    {
        int iHashCode = listener.hashCode() ;

        if (listener instanceof TerminalConnectionListener) {
            JNI_addCallListener_term(m_lHandle, (TerminalConnectionListener) listener, iHashCode, ulExcludeFilter) ;
        } else if (listener instanceof ConnectionListener) {
            JNI_addCallListener_conn(m_lHandle, (ConnectionListener) listener, iHashCode, ulExcludeFilter) ;
        } else {
            JNI_addCallListener_call(m_lHandle, listener, iHashCode, ulExcludeFilter) ;
        }
    }



    /**
     * Add a Call Listener
     */
    public void removeCallListener(CallListener listener)
    {
        JNI_removeCallListener(m_lHandle, listener, listener.hashCode()) ;
    }


    /**
     * currently not supported - give me the list of Terminal Listeners
     */
    public CallListener[] getCallListeners()
    {
        return null ;
    }



    public TerminalCapabilities getCapabilities()
    {
        return new PtTerminalCapabilities() ;
    }

    public TerminalCapabilities getTerminalCapabilities(Terminal terminal, Address address)
        throws InvalidArgumentException, PlatformException
    {
        return new PtTerminalCapabilities() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

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
// Inner classes
////
    class icComponentGroup implements ComponentGroup
    {
        protected Component[] m_comps ;


        public icComponentGroup(PtComponent[] comps)
        {
            m_comps = comps ;
        }


        public int getType()
        {
            return PHONE_SET ;
        }


        public String getDescription()
        {
            return null ;
        }


        public Component[] getComponents()
        {
            return m_comps ;
        }


        public boolean activate()
        {
            return false ;
        }


        public boolean deactivate()
        {
            return false ;
        }


        public boolean activate(Address address) throws InvalidArgumentException
        {
            return false ;
        }


        public boolean deactivate(Address address) throws InvalidArgumentException
        {
            return false ;
        }


        public ComponentGroupCapabilities getCapabilities()
        {
            return new PtComponentGroupCapabilities() ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected static native long    JNI_getProvider(String strUser, String strPassword, String strServer) ;
    protected static native long    JNI_getTerminal(long lHandle, String strName) ;
    protected static native long[]  JNI_getComponents(long lHandle) ;
    protected static native long[]  JNI_getComponentGroups(long lHandle) ;
    protected static native long[]  JNI_getAddresses(long lHandle) ;

    protected static native void    JNI_addTerminalListener(long lHandle, TerminalListener listener, long lFilter) ;
    protected static native void    JNI_removeTerminalListener(long lHandle, TerminalListener listener) ;

    protected static native void    JNI_addCallListener_call(long lHandle, CallListener listener, int iHashCode, long ulExcludeFilter) ;
    protected static native void    JNI_addCallListener_conn(long lHandle, CallListener listener, int iHashCode, long ulExcludeFilter) ;
    protected static native void    JNI_addCallListener_term(long lHandle, CallListener listener, int iHashCode, long ulExcludeFilter) ;
    protected static native void    JNI_removeCallListener(long lHandle, CallListener listener, int iHashCode) ;
    protected static native String  JNI_getName(long lHandle) ;
    protected static native void    JNI_finalize(long lHandle) ;
    protected static native void    JNI_setCodecCPULimit(long lHandle, int iLimit) ;

}
