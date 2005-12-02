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

import org.sipfoundry.telephony.callcontrol.* ;
import org.sipfoundry.telephony.capabilities.* ;
import org.sipfoundry.telephony.callcontrol.capabilities.* ;

import java.util.* ;

import javax.telephony.* ;
import javax.telephony.capabilities.* ;

import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.ShellApp;
import org.sipfoundry.sipxphone.hook.* ;
import org.sipfoundry.util.SysLog;
/**
 * Pingtel Implementation of a JTAPI Call
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PtCall extends PtWrappedObject implements Call
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** reference to our local terminal connection, so we don't have to keep looking it up! */
    protected TerminalConnection m_localTC = null ;
    /** reference to our local connection, so we don't have to keep looking it up! */
    protected Connection m_localConnection = null ;
    /** Our call id */
    protected String m_strCallID = null ;

    /** list of native call listeners */
    protected Vector m_vCallListeners = new Vector(3) ;
    /** list of native connection listeners */
    protected Vector m_vConnectionListeners = new Vector(3) ;
    /** list of native terminal connection listeners */
    protected Vector m_vTerminalConnectionListeners = new Vector(3) ;

    /** list of bridged conferences */
    protected static Vector s_vBridgedConferences = new Vector(4) ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * JNI Constructor
     */
    public PtCall(long lHandle)
    {
        super(lHandle) ;
    }


    /**
     * get all connections associated with this call
     */
    public Connection[] getConnections()
    {

//System.out.println("*********************************************") ;
//Thread.dumpStack() ;
//System.out.println("*********************************************") ;

        long lConnections[] = JNI_getConnections(m_lHandle) ;

        PtConnection connections[] = new PtConnection[lConnections.length] ;
        for (int i=0; i<lConnections.length; i++) {
            connections[i] = PtConnection.fromNativeObject(lConnections[i]) ;
        }

        return connections ;
    }


    /** not implemented */
    public Provider getProvider()
    {
        return null ;
    }

    /** not implemented */
    public int getState()
    {
        return Call.IDLE ;
    }

    /**
     * connected to the target
     * @param strOutboundLine is your outbound line address you are using to make this call.
     * If it is null, the default outbound line is used.
     */
    public Connection[] connect(Terminal origterm, Address origaddr, String dialedDigits)
            throws ResourceUnavailableException, PrivilegeViolationException, InvalidPartyException, InvalidArgumentException, InvalidStateException, MethodNotSupportedException
    {
        PtTerminal   ptTerminal = (PtTerminal) origterm ;
        PtAddress    ptLocalAddress = (PtAddress) origaddr ;
        String       strSIPURL = resolveDialString(dialedDigits) ;
        PtConnection connections[] = null ;


        try {
            long lConnections[] = JNI_connect(m_lHandle, ptTerminal.getHandle(), ptLocalAddress.getHandle(),  strSIPURL) ;

            connections= new PtConnection[lConnections.length] ;
            for (int i=0; i<lConnections.length; i++) {
                connections[i] = PtConnection.fromNativeObject(lConnections[i]) ;
            }
        } catch (PtJNIException e) {
            SysLog.log(e) ;

            switch (e.getStatus()) {
                case PtJNIException.PT_INVALID_SIP_URL:
                    throw new InvalidPartyException(InvalidPartyException.DESTINATION_PARTY) ;
                case PtJNIException.PT_INVALID_SIP_DIRECTORY_SERVER:
                    throw new InvalidStateException(ptLocalAddress, InvalidStateException.ADDRESS_OBJECT, 0) ;
                case PtJNIException.PT_INVALID_IP_ADDRESS:
                    throw new InvalidPartyException(InvalidPartyException.DESTINATION_PARTY) ;
                default:
                    throw new IllegalStateException("Unhandled Error (PtStatus=" + e.getStatus() + ")") ;
            }
        }

        return connections ;
    }


    /** not implemented */
    public void addObserver(CallObserver observer)
            throws ResourceUnavailableException, MethodNotSupportedException
    {
        throw new MethodNotSupportedException() ;
    }


    /** not implemented */
    public CallObserver[] getObservers()
    {
        return null ;
    }


    /** not implemented */
    public void removeObserver(CallObserver observer)
    {
    }


    /** not implemented */
    public CallCapabilities getCapabilities(Terminal terminal, Address address)
            throws InvalidArgumentException
    {
        return new PtCallControlCallCapabilities() ;
    }


    /** not implemented */
    public CallCapabilities getCallCapabilities(Terminal term, Address addr)
            throws InvalidArgumentException, PlatformException
    {
        return new PtCallControlCallCapabilities() ;
    }


    public void addCallListener(CallListener listener)
            throws ResourceUnavailableException, MethodNotSupportedException
    {
        int iHashCode = listener.hashCode() ;

        if (listener instanceof TerminalConnectionListener) {
            long lNativeListener = JNI_addCallListener_term(m_lHandle, (TerminalConnectionListener) listener, iHashCode, 0) ;
            if (lNativeListener != 0)
                m_vTerminalConnectionListeners.addElement(new Long(lNativeListener)) ;
        } else if (listener instanceof ConnectionListener) {
            long lNativeListener = JNI_addCallListener_conn(m_lHandle, (ConnectionListener ) listener, iHashCode, 0) ;
            if (lNativeListener != 0)
                m_vConnectionListeners.addElement(new Long(lNativeListener)) ;
        } else {
            long lNativeListener = JNI_addCallListener_call(m_lHandle, listener, iHashCode, 0) ;
            if (lNativeListener != 0)
                m_vCallListeners.addElement(new Long(lNativeListener)) ;
        }
    }


    public void addCallListener(CallListener listener, long ulExcludeFitler)
            throws ResourceUnavailableException, MethodNotSupportedException
    {
        int iHashCode = listener.hashCode() ;

        if (listener instanceof TerminalConnectionListener) {
            long lNativeListener = JNI_addCallListener_term(m_lHandle, (TerminalConnectionListener) listener, iHashCode, ulExcludeFitler) ;
            if (lNativeListener != 0)
                m_vTerminalConnectionListeners.addElement(new Long(lNativeListener)) ;
        } else if (listener instanceof ConnectionListener) {
            long lNativeListener = JNI_addCallListener_conn(m_lHandle, (ConnectionListener ) listener, iHashCode, ulExcludeFitler) ;
            if (lNativeListener != 0)
                m_vConnectionListeners.addElement(new Long(lNativeListener)) ;
        } else {
            long lNativeListener = JNI_addCallListener_call(m_lHandle, listener, iHashCode, ulExcludeFitler) ;
            if (lNativeListener != 0)
                m_vCallListeners.addElement(new Long(lNativeListener)) ;
        }
    }



    /** not implemented */
    public CallListener[] getCallListeners()
    {
        return null ;
    }


    public void removeCallListener(CallListener listener)
    {
        int iHashCode = listener.hashCode() ;

        JNI_removeCallListener(m_lHandle, listener, iHashCode) ;
    }


    /**
     * create a PtCall object from the native object handle
     */
    public static PtCall fromNativeObject(long lHandle)
    {
        return new PtCallControlCall(lHandle) ;
    }


    /**
     * what is the call id of this call?
     *
     * @NOTE This string gets cached
     */
    public String getCallID()
    {
        if (m_strCallID == null) {
            m_strCallID = JNI_getCallID(m_lHandle) ;
        }

        return m_strCallID;
    }


    /**
     * helper routine that looks through the world and finds
     * the local terminal connection
     *
     * @NOTE Local Terminal Connection gets caches
     */
    public TerminalConnection getLocalTerminalConnection()
    {
        if (m_localTC == null) {
            Connection connections[] = getConnections() ;

            // look through all of our connections
            for (int i=0; (i<connections.length) && (m_localTC == null); i++) {
                TerminalConnection termConnections[] = connections[i].getTerminalConnections() ;
                // and each of the terminal connections
                for (int j=0;j<termConnections.length; j++) {
                    if (termConnections[j] instanceof PtTerminalConnection) {
                        if (((PtTerminalConnection) termConnections[j]).isLocal()) {
                            m_localConnection = connections[i] ;
                            m_localTC = termConnections[j] ;
                            break ;
                        }
                    }
                }
            }
        }
        return m_localTC ;
    }


    /**
     * helper routine that looks through the world and finds
     * the local connection
     *
     * @NOTE Local Connection gets cached
     */
    public Connection getLocalConnection()
    {
        if (m_localConnection == null) {
            Connection connections[] = getConnections() ;

            // look through all of our connections
            for (int i=0; (i<connections.length) && (m_localConnection == null); i++) {
                TerminalConnection termConnections[] = connections[i].getTerminalConnections() ;
                // and each of the terminal connections
                for (int j=0;j<termConnections.length; j++) {
                    if (termConnections[j] instanceof PtTerminalConnection) {
                        if (((PtTerminalConnection) termConnections[j]).isLocal()) {
                            m_localConnection = connections[i] ;
                            m_localTC = termConnections[j] ;
                            break ;
                        }
                    }
                }
            }
        }
        return m_localConnection ;
    }


    /**
     * Helper routine that looks through all of the TC and finds the
     * TC with the match address.
     */
    public TerminalConnection getTerminalConnection(String strAddress)
    {
        TerminalConnection tc = null ;
        Connection connections[] = getConnections() ;

        // look through all of our connections
        for (int i=0; (i<connections.length) && (tc==null); i++)
        {
            TerminalConnection termConnections[] = connections[i].getTerminalConnections() ;
            // and each of the terminal connections
            for (int j=0;(j<termConnections.length) && (tc==null); j++)
            {
                Connection check = connections[i] ;

                if (check != null)
                {
                    String strCheckAddress = check.getAddress().getName() ;

                    if (PtAddress.areSipAddressesEquivalent(strAddress, strCheckAddress))
                    {
                        tc = termConnections[j] ;
                    }
                }
            }
        }
        return tc ;
    }


    public String toString()
    {
        return getCallID() ;
    }

    /**
     * Calls are a little annoying and require a special equals method.  We
     * can determine if calls are equal only by looking that the native
     * call id.
     */
    public boolean equals(Object obj)
    {
        boolean bRC = false ;

        PtCall callObj = (PtCall) obj ;
        if (callObj != null) {

            if (getCallID().equals(callObj.getCallID())) {
                bRC = true ;
            }
        }

        return bRC ;
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
        /*
         * Remove defunct listeners
         */

        // Remove Call Listeners
        for (int i=0; i<m_vCallListeners.size(); i++) {
            Long l = (Long) m_vCallListeners.elementAt(i) ;
            JNI_finalizeCallListener(l.longValue()) ;
        }
        m_vCallListeners.removeAllElements() ;

        // Remove Connection Listeners
        for (int i=0; i<m_vConnectionListeners.size(); i++) {
            Long l = (Long) m_vConnectionListeners.elementAt(i) ;
            JNI_finalizeConnectionListener(l.longValue()) ;
        }
        m_vConnectionListeners.removeAllElements() ;

        // Remove Terminal Connection Listeners
        for (int i=0; i<m_vTerminalConnectionListeners.size(); i++) {
            Long l = (Long) m_vTerminalConnectionListeners.elementAt(i) ;
            JNI_finalizeTerminalConnectionListener(l.longValue()) ;
        }
        m_vTerminalConnectionListeners.removeAllElements() ;


        /*
         * Finalize native java object
         */
        JNI_finalize(m_lHandle) ;
        m_lHandle = 0 ;
    }


    public void disablePremiumSound(String strCallId)
    {
        JNI_disablePremiumSound(strCallId, 0) ;
    }


    /**
     * Convenience method that places various terminal connections on hold
     */
    public void hold(boolean bLocalHold)
    {
        JNI_hold(getHandle(), bLocalHold) ;
    }


    /**
     * Convenience method that takes all of the associated terminal connections
     * off hold
     *
     * @TODO move this method into somesort of extension
     */
    public void unhold()
    {
        unhold(false) ;
    }


    /**
     * Convenience method that takes all terminal connections off hold.
     *
     * @param bLocalOnly If true, this method will only take the local TC off
     *        hold, otherwise, it will take all TCs off hold.
     *
     * @TODO move this method into somesort of extension
     */
    public void unhold(boolean bLocalOnly)
    {
        if (Shell.getXpressaSettings().getProductID() == XpressaSettings.PRODUCT_XPRESSA)
        {
            if (s_vBridgedConferences.size() > 0)
            {
                // HACK: hold/unhold should be moved to PtCallControlCall
                PtCallControlCall call = (PtCallControlCall) this ;
                call.setCodecCPULimit(PtCallControlCall.CODEC_CPU_LOW, false) ;
            }
        }
        JNI_unhold(getHandle(), !bLocalOnly) ;
    }



//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Evaluate the dialed string and match against known digit maps and what
     * not.
     */
    protected String resolveDialString(String strDialString)
    {
        HookManager hookManager = Shell.getHookManager() ;

        MatchDialplanHookData data = new MatchDialplanHookData(strDialString) ;
        hookManager.executeHook(HookManager.HOOK_MATCH_DIALPLAN, data) ;

        switch (data.getMatchState()) {
            case MatchDialplanHookData.MATCH_FAILURE:
                break ;
            case MatchDialplanHookData.MATCH_SUCCESS:
            case MatchDialplanHookData.MATCH_TIMEOUT:
                if (data.getAddress() != null) {
                    strDialString = data.getAddress() ;
                }
                break ;
        }

        return strDialString ;
    }


//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected native static long[] JNI_connect(long lhandle, long lTerminal, long lAddress, String strSIPURL)
        throws PtJNIException ;
    protected native static long   JNI_addCallListener_call(long lHandle, CallListener listener, int iHashCode, long ulExcludeFilter) ;
    protected native static long   JNI_addCallListener_conn(long lHandle, ConnectionListener listener, int iHashCode, long ulExcludeFilter) ;
    protected native static long   JNI_addCallListener_term(long lHandle, TerminalConnectionListener listener, int iHashCode, long ulExcludeFilter) ;
    protected native static void   JNI_removeCallListener(long lHandle, CallListener listener, int iHashCode) ;
    protected native static long[] JNI_getConnections(long lHandle) ;
    protected native static String JNI_getCallID(long lHandle) ;

    protected native static void   JNI_hold(long lHandle, boolean bBrideParticipants) ;
    protected native static void   JNI_unhold(long lHandle, boolean bRemoteParticipants) ;

    protected native static void   JNI_finalize(long lHandle) ;

    protected native static void   JNI_finalizeCallListener(long lHandle) ;
    protected native static void   JNI_finalizeConnectionListener(long lHandle) ;
    protected native static void   JNI_finalizeTerminalConnectionListener(long lHandle) ;

    protected native static void   JNI_disablePremiumSound(String strCallId, int unused) ;
}
