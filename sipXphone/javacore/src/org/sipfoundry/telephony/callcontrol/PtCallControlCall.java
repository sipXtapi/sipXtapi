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

package org.sipfoundry.telephony.callcontrol ;

import javax.telephony.* ;
import javax.telephony.callcontrol.* ;
import javax.telephony.capabilities.* ;
import javax.telephony.callcontrol.capabilities.* ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.callcontrol.capabilities.* ;

import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.sys.util.PingerInfo ;

import org.sipfoundry.util.SysLog;
import org.sipfoundry.sipxphone.sys.Shell ;
import org.sipfoundry.sipxphone.sys.app.ShellApp ;
import org.sipfoundry.sipxphone.sys.XpressaSettings ;

public class PtCallControlCall extends PtCall implements CallControlCall
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final int CODEC_CPU_LOW = 0 ;
    public static final int CODEC_CPU_HIGH = 1 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Address m_callingAddress = null ;
    private Address m_calledAddress = null ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public PtCallControlCall(long lHandle)
    {
        super(lHandle) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////


    /* JTAPI EXTENSION */
    public void forceCodecRenegotiation()
    {
        JNI_forceCodecRenegotiation(m_lHandle, 0) ;
    }

    /* JTAPI EXTENSION */
    public void setCodecCPULimit(int iLimit, boolean bRenegotiate)
    {
        JNI_setCodecCPULimit(m_lHandle, iLimit, bRenegotiate) ;
    }

    /* JTAPI EXTENSION */
    public int getCodecCPUCost()
    {
        return JNI_getCodecCPUCost(m_lHandle, 0) ;
    }

    /* JTAPI EXTENSION */
    public int getCodecCPULimit()
    {
        return JNI_getCodecCPULimit(m_lHandle, 0) ;
    }

    public Address getCallingAddress()
    {
        if (m_callingAddress == null) {
            long lAddress = JNI_getCallingAddress(m_lHandle) ;
            if (lAddress != 0) {
                m_callingAddress = PtAddress.fromNativeObject(lAddress) ;
            }
        }

        return m_callingAddress ;
    }


    public Address getCalledAddress()
    {
        if (m_calledAddress == null) {
            long lAddress = JNI_getCalledAddress(m_lHandle) ;
            if (lAddress != 0) {
                m_callingAddress = PtAddress.fromNativeObject(lAddress) ;
            }
        }

        return m_calledAddress ;
    }


    public Terminal getCallingTerminal()
    {
        Terminal terminal = null ;
        long lTerminal = JNI_getCallingTerminal(m_lHandle) ;
        if (lTerminal != 0) {
            terminal = new PtTerminal(lTerminal) ;
        }

        return terminal ;
    }


    public void drop()
        throws InvalidStateException, MethodNotSupportedException, PrivilegeViolationException, ResourceUnavailableException
    {
        if (Shell.getXpressaSettings().getProductID() == XpressaSettings.PRODUCT_XPRESSA)
        {
            if (s_vBridgedConferences.contains(getCallID()))
            {
                s_vBridgedConferences.removeElement(getCallID()) ;
                if (s_vBridgedConferences.size() == 0)
                {
                    PtTerminal terminal = (PtTerminal) ShellApp.getInstance().getTerminal() ;
                    terminal.setCodecCPULimit(PtCallControlCall.CODEC_CPU_HIGH) ;
                }
            }
        }

        JNI_drop(getHandle()) ;
    }


    /** not yet implemented */
    public Address getLastRedirectedAddress()
    {
        return null ;
    }


    public Connection addParty(String newParty)
            throws InvalidStateException, InvalidPartyException, MethodNotSupportedException, PrivilegeViolationException, ResourceUnavailableException
    {
        PingerInfo pingerInfo = PingerInfo.getInstance() ;
        Connection connection = null ;

        // If this is an xpressa; drop us down to a low cpu codec
        XpressaSettings settings = Shell.getXpressaSettings() ;
        if (settings.getProductID() == XpressaSettings.PRODUCT_XPRESSA)
        {
            setCodecCPULimit(PtCallControlCall.CODEC_CPU_LOW, true);
            String strCallId = getCallID() ;
            if (!s_vBridgedConferences.contains(strCallId))
            {
                s_vBridgedConferences.addElement(strCallId) ;

                PtTerminal terminal = (PtTerminal) ShellApp.getInstance().getTerminal() ;
                terminal.setCodecCPULimit(PtCallControlCall.CODEC_CPU_LOW) ;
            }
        }

        // Abort if the number of connections >= the max number of connections
        if (pingerInfo.getSimultaneousConnections() >= pingerInfo.getMaxSimultaneousConnections()) {
            System.out.println("MAX CONNECTIONS REACHED") ;
            System.out.println("Simultaneous Connections: " + pingerInfo.getSimultaneousConnections()) ;
            System.out.println("         Max Connections: " + pingerInfo.getMaxSimultaneousConnections()) ;

            throw new ResourceUnavailableException(ResourceUnavailableException.UNSPECIFIED_LIMIT_EXCEEDED, "max simultaneous connections exceeded") ;
        } else {
            String strSIPURL = resolveDialString(newParty) ;

            try {
                long lConnection = JNI_addParty(getHandle(), strSIPURL) ;
                if (lConnection != 0) {
                    connection = PtConnection.fromNativeObject(lConnection) ;
                }
            } catch (PtJNIException e) {
                SysLog.log(e) ;

                switch (e.getStatus()) {
                    case PtJNIException.PT_INVALID_SIP_URL:
                        throw new InvalidPartyException(InvalidPartyException.DESTINATION_PARTY) ;
                    case PtJNIException.PT_INVALID_SIP_DIRECTORY_SERVER:
                        throw new InvalidStateException(null, InvalidStateException.ADDRESS_OBJECT, 0) ;
                    case PtJNIException.PT_INVALID_IP_ADDRESS:
                        throw new InvalidPartyException(InvalidPartyException.DESTINATION_PARTY) ;
                    default:
                        throw new IllegalStateException("Unhandled Error (PtStatus=" + e.getStatus()+")") ;
                }
            }
        }
        return connection ;
    }

    /** not yet implemented */
    public Connection offHook(Address origaddress,
                          Terminal origterminal)
                   throws InvalidStateException,
                          MethodNotSupportedException,
                          PrivilegeViolationException,
                          ResourceUnavailableException
    {
        throw new MethodNotSupportedException() ;
    }


    /** not yet implemented */
    public void conference(Call otherCall)
                throws InvalidStateException,
                       InvalidArgumentException,
                       MethodNotSupportedException,
                       PrivilegeViolationException,
                       ResourceUnavailableException
    {
        throw new MethodNotSupportedException() ;
    }


    public void transfer(Call otherCall)
              throws InvalidStateException,
                     InvalidArgumentException,
                     InvalidPartyException,
                     MethodNotSupportedException,
                     PrivilegeViolationException,
                     ResourceUnavailableException
    {
        try {
            JNI_transfer_toCall(m_lHandle, ((PtCall) otherCall).getHandle()) ;
        } catch (PtJNIException e) {
            SysLog.log(e) ;

            switch (e.getStatus()) {
                case PtJNIException.PT_INVALID_SIP_URL:
                    throw new InvalidPartyException(InvalidPartyException.DESTINATION_PARTY) ;
                case PtJNIException.PT_INVALID_SIP_DIRECTORY_SERVER:
                    throw new InvalidStateException(null, InvalidStateException.ADDRESS_OBJECT, 0) ;
                case PtJNIException.PT_INVALID_IP_ADDRESS:
                    throw new InvalidPartyException(InvalidPartyException.DESTINATION_PARTY) ;
                default:
                    throw new IllegalStateException("Unhandled Error (PtStatus=" + e.getStatus()+")") ;
            }
        }
    }


    public Connection transfer(java.lang.String address)
                    throws InvalidArgumentException,
                           InvalidStateException,
                           InvalidPartyException,
                           MethodNotSupportedException,
                           PrivilegeViolationException,
                           ResourceUnavailableException
    {
        Connection connection = null ;

        try {
            long lConnection = JNI_transfer_toAddress(m_lHandle, resolveDialString(address)) ;

            if (lConnection != 0) {
                connection = PtConnection.fromNativeObject(lConnection) ;
            }
        } catch (PtJNIException e) {
            SysLog.log(e) ;

            switch (e.getStatus()) {
                case PtJNIException.PT_INVALID_SIP_URL:
                    throw new InvalidPartyException(InvalidPartyException.DESTINATION_PARTY) ;
                case PtJNIException.PT_INVALID_SIP_DIRECTORY_SERVER:
                    throw new InvalidStateException(null, InvalidStateException.ADDRESS_OBJECT, 0) ;
                case PtJNIException.PT_INVALID_IP_ADDRESS:
                    throw new InvalidPartyException(InvalidPartyException.DESTINATION_PARTY) ;
                default:
                    throw new IllegalStateException("Unhandled Error (PtStatus=" + e.getStatus()+")") ;
            }
        }

        return connection ;
    }

    /** not yet implemented */
    public void setConferenceController(TerminalConnection tc)
                             throws InvalidArgumentException,
                                    InvalidStateException,
                                    MethodNotSupportedException,
                                    ResourceUnavailableException
    {
        throw new MethodNotSupportedException() ;
    }


    /** not yet implemented */
    public TerminalConnection getConferenceController()
    {
        return null ;
    }


    /** not yet implemented */
    public void setTransferController(TerminalConnection tc)
                           throws InvalidArgumentException,
                                  InvalidStateException,
                                  MethodNotSupportedException,
                                  ResourceUnavailableException
    {
        throw new MethodNotSupportedException() ;
    }

    /** not yet implemented */
    public TerminalConnection getTransferController()
    {
        return null ;
    }

    /** not yet implemented */
    public void setConferenceEnable(boolean enable)
                         throws InvalidArgumentException,
                                InvalidStateException,
                                MethodNotSupportedException,
                                PrivilegeViolationException
    {
        throw new MethodNotSupportedException() ;
    }

    /** not yet implemented */
    public boolean getConferenceEnable()
    {
        return false ;
    }

    /** not yet implemented */
    public void setTransferEnable(boolean enable)
                       throws InvalidArgumentException,
                              InvalidStateException,
                              MethodNotSupportedException,
                              PrivilegeViolationException
    {
        throw new MethodNotSupportedException() ;
    }

    /** not yet implemented */
    public boolean getTransferEnable()
    {
        return false ;
    }

    public Connection[] consult(TerminalConnection tc, java.lang.String dialedDigits)
                     throws InvalidStateException,
                            InvalidArgumentException,
                            MethodNotSupportedException,
                            ResourceUnavailableException,
                            PrivilegeViolationException,
                            InvalidPartyException
    {
        PtConnection connections[] = new PtConnection[0] ;
        long lTerminalConnection = ((PtTerminalConnection) tc).getHandle() ;

        try
        {
            // Start the consult
            long lConnections[] = JNI_consult(m_lHandle, lTerminalConnection,
                    resolveDialString(dialedDigits)) ;

            // Convert the arguments into java objects
            connections = new PtConnection[lConnections.length] ;
            for (int i=0; i<lConnections.length; i++) {
                connections[i] = PtConnection.fromNativeObject(lConnections[i]) ;
            }
        }
        catch (PtJNIException e)
        {
            SysLog.log(e) ;

            switch (e.getStatus())
            {
                case PtJNIException.PT_INVALID_SIP_URL:
                    throw new InvalidPartyException(InvalidPartyException.DESTINATION_PARTY) ;
                case PtJNIException.PT_INVALID_SIP_DIRECTORY_SERVER:
                    throw new InvalidStateException(null, InvalidStateException.ADDRESS_OBJECT, 0) ;
                case PtJNIException.PT_INVALID_IP_ADDRESS:
                    throw new InvalidPartyException(InvalidPartyException.DESTINATION_PARTY) ;
                default:
                    throw new IllegalStateException("Unhandled Error (PtStatus=" + e.getStatus()+")") ;
            }
        }

        return connections ;
    }

    /** not yet implemented */
    public Connection consult(TerminalConnection tc)
                   throws InvalidStateException,
                          InvalidArgumentException,
                          MethodNotSupportedException,
                          ResourceUnavailableException,
                          PrivilegeViolationException
    {
        throw new MethodNotSupportedException() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected static native long JNI_getCallingAddress(long lHandle) ;
    protected static native long JNI_getCalledAddress(long lHandle) ;
    protected static native long JNI_getCallingTerminal(long lHandle) ;
    protected static native void JNI_drop(long lHandle) ;

    protected static native long JNI_transfer_toAddress(long lHandle, String strAddress)
        throws PtJNIException ;
    protected static native void JNI_transfer_toCall(long lHandle, long lCallHandle)
        throws PtJNIException ;
    protected static native long JNI_addParty(long lHandle, String strAddress)
        throws PtJNIException ;

    protected static native long[] JNI_consult(long lHandle, long lTCHandle, String strAddress)
        throws PtJNIException ;

    protected static native void JNI_forceCodecRenegotiation(long lHandle, int unused) ;
    protected static native void JNI_setCodecCPULimit(long lHandle, int limit, boolean bAutoRenegotiate) ;
    protected static native int  JNI_getCodecCPUCost(long lHandle, int unused) ;
    protected static native int  JNI_getCodecCPULimit(long lHandle, int unused) ;
}

