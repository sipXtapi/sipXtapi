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
import javax.telephony.callcontrol.* ;
import javax.telephony.capabilities.* ;
import org.sipfoundry.sip.SipSession;


import org.sipfoundry.telephony.callcontrol.* ;
import org.sipfoundry.telephony.capabilities.* ;
import org.sipfoundry.telephony.callcontrol.capabilities.* ;


public class PtConnection extends PtWrappedObject implements Connection
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    protected final int PTAPI_IDLE              = 0x50 ;
    protected final int PTAPI_OFFERED           = 0x51 ;
    protected final int PTAPI_QUEUED            = 0x52 ;
    protected final int PTAPI_ALERTING          = 0x53 ;
    protected final int PTAPI_INITIATED         = 0x54 ;
    protected final int PTAPI_DIALING           = 0x55 ;
    protected final int PTAPI_NETWORK_REACHED   = 0x56 ;
    protected final int PTAPI_NETWORK_ALERTING  = 0x57 ;
    protected final int PTAPI_ESTABLISHED       = 0x58 ;
    protected final int PTAPI_DISCONNECTED      = 0x59 ;
    protected final int PTAPI_FAILED            = 0x5A ;
    protected final int PTAPI_UNKNOWN           = 0x5B ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Call    m_call = null ;
    private Address m_address = null ;

    public PtConnection(long lHandle)
    {
        super(lHandle) ;
    }


    public int getState()
    {
        return toCallControlJTAPIEventId(JNI_getState(m_lHandle)) ;
    }


    public Call getCall()
    {
        if (m_call == null) {
            long lCall = JNI_getCall(m_lHandle) ;
            if (lCall != 0) {
                m_call = PtCall.fromNativeObject(lCall) ;
            }
        }
        return m_call ;
    }


    public Address getAddress()
    {
        if (m_address == null) {
            long lHandle = JNI_getAddress(m_lHandle) ;
            if (lHandle != 0) {
                m_address = new PtAddress(lHandle) ;
            }
        }

        return m_address ;
    }


    public TerminalConnection[] getTerminalConnections()
    {
//System.out.println("*********************************************") ;
//Thread.dumpStack() ;
//System.out.println("*********************************************") ;

        long lConnections[] = JNI_getTerminalConnections(m_lHandle) ;

        PtTerminalConnection connections[] = new PtTerminalConnection[lConnections.length] ;
        for (int i=0; i<lConnections.length; i++) {
            connections[i] = PtTerminalConnection.fromNativeObject(lConnections[i]) ;
        }
        return connections ;
    }


    /**
     * Get the fully qualifed URI for the "To" field of this connection. This
     * URI may be different from regular addresses and include tags that are
     * normally stripped before users can see them.
     * <br><br>
     * NOTE: This is a Pingtel Extension
     */
    public String getToURI()
    {
        return JNI_getToURI(m_lHandle) ;
    }


    /**
     * Get the fully qualifed URI for the "From" field of this connection.
     * This URI may be different from regular addresses and include tags
     * that are normally stripped before developers can see them.
     * <br><br>
     * NOTE: This is a Pingtel Extension
     */
    public String getFromURI()
    {
        return JNI_getFromURI(m_lHandle) ;
    }


    /**
     * Get the SipSession for this connection.
     * <br><br>
     * NOTE: This is a Pingtel Extension
     */
    public SipSession getSession()
    {
        return JNI_getSession(m_lHandle) ;
    }


    public void disconnect()
            throws PrivilegeViolationException, ResourceUnavailableException, MethodNotSupportedException, InvalidStateException
    {
        JNI_disconnect(m_lHandle) ;
    }

    /** not implemented */
    public ConnectionCapabilities getCapabilities()
    {
        return new PtCallControlConnectionCapabilities() ;
    }

    /** not implemented */
    public ConnectionCapabilities getConnectionCapabilities(Terminal terminal, Address address)
            throws InvalidArgumentException, PlatformException
    {
        return new PtCallControlConnectionCapabilities() ;
    }


    /**
     * create an appropriate connection object.  This figures out if the
     * native handle is a CallControlConnection or just a Connection and
     * creates the proper java object.
     */
    public static PtConnection fromNativeObject(long lHandle)
    {
        PtConnection connection = null ;

        if (lHandle != 0) {
            connection = new PtCallControlConnection(lHandle) ;
        }

        return connection ;
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



    public String toJTAPIEventString(int iJTAPIEvent)
    {
        String strRC = "Unknown State" ;

        switch (iJTAPIEvent)
        {
            case CallControlConnection.IDLE:
            case Connection.IDLE:
                strRC = "IDLE" ;
                break ;
            case Connection.INPROGRESS:
                strRC = "INPROGRESS" ;
                break ;
            case CallControlConnection.QUEUED:
                strRC = "QUEUED" ;
                break ;
            case CallControlConnection.OFFERED:
                strRC = "OFFERED" ;
                break ;
            case Connection.CONNECTED:
                strRC = "CONNECTED" ;
                break ;
            case CallControlConnection.ALERTING:
                strRC = "ALERTING" ;
                break ;
            case CallControlConnection.INITIATED:
                strRC = "INITIATED" ;
                break ;
            case CallControlConnection.DIALING:
                strRC = "DIALING" ;
                break ;
            case CallControlConnection.NETWORK_REACHED:
                strRC = "NETWORK_REACHED" ;
                break ;
            case CallControlConnection.NETWORK_ALERTING:
                strRC = "NETWORK_ALERTING" ;
                break ;
            case CallControlConnection.ESTABLISHED:
                strRC = "ESTABLISHED" ;
                break ;
            case Connection.DISCONNECTED:
            case CallControlConnection.DISCONNECTED:
                strRC = "DISCONNECTED" ;
                break ;
            case CallControlConnection.FAILED:
            case Connection.FAILED:
                strRC = "FAILED" ;
                break ;
            case CallControlConnection.UNKNOWN:
            case Connection.UNKNOWN:
                strRC = "UNKNOWN" ;
                break ;
        }

        return strRC ;
    }



    /**
     * Converts the specified PTAPI event into a Connection level JTAPI
     * event.
     */
    protected int toJTAPIEventId(int iPTAPIEvent)
    {
        int iEvent = Connection.UNKNOWN ;

        switch (iPTAPIEvent)
        {
            case PTAPI_IDLE:
                iEvent = Connection.IDLE ;
                break ;
            case PTAPI_OFFERED:
            case PTAPI_QUEUED:
                iEvent = Connection.INPROGRESS ;
                break ;
            case PTAPI_ALERTING:
                iEvent = Connection.ALERTING ;
                break ;
            case PTAPI_INITIATED:
            case PTAPI_DIALING:
            case PTAPI_NETWORK_REACHED:
            case PTAPI_NETWORK_ALERTING:
            case PTAPI_ESTABLISHED:
                iEvent = Connection.INPROGRESS ;
                break ;
            case PTAPI_DISCONNECTED:
                iEvent = Connection.DISCONNECTED ;
                break ;
            case PTAPI_FAILED:
                iEvent = Connection.FAILED ;
                break ;
            case PTAPI_UNKNOWN:
                iEvent = Connection.UNKNOWN ;
                break ;
        }
        return iEvent ;
    }



    /**
     * Converts the specified PTAPI event into a Call Control Connection
     * level JTAPI event.
     */
    protected int toCallControlJTAPIEventId(int iPTAPIEvent)
    {
        int iEvent = CallControlConnection.UNKNOWN ;

        switch (iPTAPIEvent)
        {
            case PTAPI_IDLE:
                iEvent = CallControlConnection.IDLE ;
                break ;
            case PTAPI_QUEUED:
                iEvent = CallControlConnection.QUEUED ;
                break ;
            case PTAPI_OFFERED:
                iEvent = CallControlConnection.OFFERED ;
                break ;
            case PTAPI_ALERTING:
                iEvent = CallControlConnection.ALERTING ;
                break ;
            case PTAPI_INITIATED:
                iEvent = CallControlConnection.INITIATED ;
                break ;
            case PTAPI_DIALING:
                iEvent = CallControlConnection.DIALING ;
                break ;
            case PTAPI_NETWORK_REACHED:
                iEvent = CallControlConnection.NETWORK_REACHED ;
                break ;
            case PTAPI_NETWORK_ALERTING:
                iEvent = CallControlConnection.NETWORK_ALERTING ;
                break ;
            case PTAPI_ESTABLISHED:
                iEvent = CallControlConnection.ESTABLISHED ;
                break ;
            case PTAPI_DISCONNECTED:
                iEvent = CallControlConnection.DISCONNECTED ;
                break ;
            case PTAPI_FAILED:
                iEvent = CallControlConnection.FAILED ;
                break ;
            case PTAPI_UNKNOWN:
                iEvent = CallControlConnection.UNKNOWN ;
                break ;
        }

        return iEvent ;
    }


    public String toString()
    {
        int iState = getState() ;
        return "connection: " + getAddress().getName() + ", state=" + iState + "/" + toJTAPIEventString(iState) ;
    }


    public boolean isLocal()
    {
        boolean bLocal = false ;

        TerminalConnection tc[] = getTerminalConnections() ;
        for (int i=0;i<tc.length; i++) {
            if (((PtTerminalConnection) tc[i]).isLocal()) {
                bLocal = true ;
                break ;
            }
        }

        return bLocal ;
    }


    /**
     * Dumps the state of this connection.
     */
    public void dump()
    {
        TerminalConnection tc[] = getTerminalConnections() ;

        System.out.println("") ;
        System.out.println("DUMP PtConnection: ") ;
        System.out.println("State: " + toJTAPIEventString(getState())) ;
        System.out.println("Address: " + getAddress().getName()) ;

        for (int i=0;i<tc.length; i++) {
            ((PtTerminalConnection) tc[i]).dump() ;
        }
    }



//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected static native boolean JNI_isCallControl(long lHandle) ;
    protected static native void    JNI_disconnect(long lHandle) ;
    protected static native long[]  JNI_getTerminalConnections(long lHandle) ;
    protected static native long    JNI_getCall(long lHandle) ;
    protected static native long    JNI_getAddress(long lHandle) ;
    protected static native void    JNI_finalize(long lHandle) ;
    protected static native int     JNI_getState(long lHandle) ;

    protected static native String  JNI_getToURI(long lHandle) ;
    protected static native String  JNI_getFromURI(long lHandle) ;
    protected static native SipSession JNI_getSession(long lHandle) ;
}
