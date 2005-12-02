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


package org.sipfoundry.sipxphone.app.conference ;

import java.awt.* ;
import java.net.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.hook.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.stapi.* ;

import javax.telephony.* ;
import javax.telephony.callcontrol.* ;
import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.callcontrol.* ;



/**
 * Data Item representing a potential or an actual conference participant.
 * This data object can take three types forms, each of with has different
 * data associated it:
 * <ul>
 *  <li>TYPE_CONNECTION: Conference participant represented by a PAddress
 *                       and the conference call</li>
 *
 *  <li>TYPE_HELDCALL: Conference participant(s) represented by a held call</li>
 *
 *  <li>TYPE_ADDRESS: Potential conference participant represented by a
 *      PAddress</li>
 * </ul>
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ConferenceCallDataItem
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    // The type of data item
    public static final int TYPE_CONNECTION = 0 ;
    public static final int TYPE_HELDCALL   = 1 ;
    public static final int TYPE_ADDRESS    = 2 ;

    // possible data item states
    public static final int STATE_MUST_LOOKUP = -1 ;
    public static final int STATE_UNAVAILABLE = 0 ;
    public static final int STATE_HELD        = 1 ;
    public static final int STATE_TRYING      = 2 ;
    public static final int STATE_RINGING     = 3 ;
    public static final int STATE_FAILED      = 4 ;
    public static final int STATE_CONNECTED   = 5 ;
    public static final int STATE_UNKNOWN     = 6 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private int m_iType ;               // Type of this data object
    private int m_iState ;              // Cached State
    private PAddress m_address ;        // Address of this participant
    private PCall m_callConference ;    // Conference call itself (if applicable)
    private PCall m_callHeld ;          // call object if this is a held call

    private String m_strRenderedName ;      // Cached pre-rendered name
    private String m_strRenderedAddress ;   // Cached pre-rendered address

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs a conference call data item with the specified particpant
     * address and conference call object.  The type set is to
     * TYPE_CONNECTION.
     *
     * @param address The address of a single conference participant
     * @param call The conference call that the specified particpant
     *             is a member of.
     */
    public ConferenceCallDataItem(PAddress address, PCall callConference)
    {
        m_iType = TYPE_CONNECTION ;
        m_iState = STATE_MUST_LOOKUP ;
        m_address = address ;
        m_callConference = callConference ;
        m_callHeld = null ;
    }


    /**
     * Constructs a conference call data item with the potential participant
     * address.  The type is set to TYPE_ADDRESS.  It is assume that this address
     * represents an individual out of a conference group.  However, the address,
     * can also represent a failed call or any sort of unavailable member.     *
     * @param address The address of a potential conference participant
     */
    public ConferenceCallDataItem(PAddress address)
    {
        m_iType = TYPE_ADDRESS ;
        m_iState = STATE_UNAVAILABLE ;
        m_address = address ;
        m_callConference = null ;
        m_callHeld = null ;
    }


    /**
     * Constructs a conference call data item with the potential participant
     * address.  The type is set to TYPE_ADDRESS.  It is assume that this address
     * represents an individual out of a conference group.  However, the address,
     * can also represent a failed call or any sort of unavailable member.
     *
     * @param address The address of a potential conference participant
     * @param iState The state of the specified address
     */
    public ConferenceCallDataItem(PAddress address, int iState)
    {
        m_iType = TYPE_ADDRESS ;
        m_iState = iState ;
        m_address = address ;
        m_callConference = null ;
        m_callHeld = null ;
    }


    /**
     * Constructs a conference call data item with the specified held call.
     * That held call will be displayed in the conference list as a potential
     * conferencee.  The type is set to TYPE_HELDCALL.
     *
     * @param callHeld The held call that may be joined into the conference.
     */
    public ConferenceCallDataItem(PCall callHeld)
    {
        m_iType = TYPE_HELDCALL ;
        m_iState = STATE_HELD ;

        m_callConference = null;
        m_callHeld = callHeld ;

        // Snag held call address
        PAddress participants[] = m_callHeld.getParticipants() ;
        m_address = participants[0] ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Is this an active conference participant?  A participant is considered
     * active if their state is: STATE_HELD, STATE_TRYING, STATE_RINGING,
     * STATE_UNKNOWN, or STATE_CONNECTED
     */
    public synchronized boolean isActive()
    {
        int iState = getState() ;

        return  ((iState == STATE_HELD) ||
                (iState == STATE_TRYING) ||
                (iState == STATE_RINGING) ||
                (iState == STATE_CONNECTED) ||
                (iState == STATE_UNKNOWN)) ;
    }


    /**
     * Get the address of the conference participant represented by this data
     * object.
     **
     * @return PAddress of the conference participant
     */
    public PAddress getAddress()
    {
        return m_address ;
    }


    /**
     * Get the type of this call data item.  See the class level comments for
     * a description of possible types.
     *
     * @return The conference call data item type.
     */
    public synchronized int getType()
    {
        return m_iType ;
    }


    /**
     * Set the type of this call data item.  This method is protected on
     * purpose and people should not be calling it.  I'm exposing this as
     * a hack because connectionHeld events are not being exposed through
     * STAPI, yet.
     */
    protected synchronized void setType(int iType)
    {
        if (m_iType != iType) {
            m_iType = iType ;
            updateState() ;
        }
    }


    /**
     * Forces this object to update it's own state.
     */
    protected synchronized void updateState()
    {
        // This only takes action if we are a connection, otherwise, we have
        // no where to look for further information.
        if (TYPE_CONNECTION == m_iType) {
            m_iState = STATE_MUST_LOOKUP ;
        }
    }



    /**
     * Set the state of this data object
     */
    protected synchronized void setState(int iState)
    {
        m_iState = iState ;
    }



    /**
     * Get the current state of this call data item.  The state depends on the
     * type of object.
     *
     * @return the state of the object or unknown if the state cannot be
     *         determinted.
     */
    public synchronized int getState()
    {

        if (m_iState == STATE_MUST_LOOKUP)
        {
            m_iState = STATE_UNKNOWN ;
            int iConnectionState = PCall.CONN_STATE_UNKNOWN ;
            switch (m_iType)
            {
                case TYPE_CONNECTION:
                    iConnectionState = m_callConference.getConnectionState(m_address) ;

                    switch (iConnectionState)
                    {
                        case PCall.CONN_STATE_CONNECTED:

                            if (m_callConference.isHeld(m_address))
                            {
                                m_iState = STATE_HELD ;
                            }
                            else
                            {
                                m_iState = STATE_CONNECTED ;
                            }
                            break ;
                        case PCall.CONN_STATE_TRYING:
                            m_iState = STATE_TRYING ;
                            break ;
                        case PCall.CONN_STATE_OUTBOUND_ALERTING:
                        case PCall.CONN_STATE_INBOUND_ALERTING:
                            m_iState = STATE_RINGING ;
                            break ;
                        case PCall.CONN_STATE_HELD:
                            m_iState = STATE_HELD ;
                            break ;
                        case PCall.CONN_STATE_FAILED:
                            m_iState = STATE_FAILED ;
                            break ;
                        case PCall.CONN_STATE_DISCONNECTED:
                            m_iState = STATE_UNAVAILABLE ;
                            break ;
                        default:
                            m_iState = STATE_UNKNOWN ;
                            break ;
                    }
                    break ;
                case TYPE_HELDCALL:
                    m_iState = STATE_HELD ;
                    break ;
                case TYPE_ADDRESS:
                    m_iState = STATE_UNAVAILABLE ;
                    break ;
            }
        }

        return m_iState ;
    }


    /**
     * Get the actually conference call that this data object is describing.
     *
     * @return PCall conference call
     */
    public PCall getConferenceCall()
    {
        return m_callConference ;
    }


    /**
     * Get the held call if the type is TYPE_HELDCALL.
     *
     * @return The held call or null if the type is not TYPE_HELDCALL
     */
    public PCall getHeldCall()
    {
        return m_callHeld ;
    }


    /**
     * Get the pre-rendered and cached participant name.  The process
     * of applying caller id ontop of a PCall can be somewhat expensive and
     * users should developers should try to cache this type of data whenever
     * possible.
     *
     * @return String name of the conference participant
     */
    public String getRenderedName()
    {
        if (m_strRenderedName == null) {
            m_strRenderedName = getDisplayName(m_address.getAddress()) ;
        }

        return m_strRenderedName ;
    }


    /**
     * Get the pre-rendered and cached participant address .  The process
     * of applying caller id ontop of a PCall can be somewhat expensive and
     * users should developers should try to cache this type of data whenever
     * possible.
     *
     * @return String address of the conference participant
     */
    public String getRenderedAddress()
    {
        if (m_strRenderedAddress == null) {
            m_strRenderedAddress = getDisplayAddress(m_address.getAddress()) ;
        }

        return m_strRenderedAddress ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * What is the display name of the passed SIP URL?  The data returned here
     * depends on the SIP URL itself.  If it contains a Display Name, that
     * string is returned, otherwise, the user id is return.  If no user name
     * is found, the SIP URL itself is returned.
     */
    protected String getDisplayName(String strSIPURL)
    {
        String strName = strSIPURL ;

        SipParser parser = new SipParser(strSIPURL) ;

        // Do we have a Display Name?
        String strDisplayName = parser.getDisplayName() ;
        if ((strDisplayName != null) && (strDisplayName.length() > 0)) {
            strName = strDisplayName ;
        } else {
            // Okay, how about a user id?
            String strUserID = parser.getUser() ;
            if ((strUserID != null) && (strUserID.length() > 0)) {
                strName = strUserID ;
            }
        }

        HookManager hookManager = Shell.getHookManager() ;
        try {
            CallerIDHookData data = new CallerIDHookData(PAddressFactory.getInstance().createAddress(strSIPURL), strName) ;
            hookManager.executeHook(HookManager.HOOK_CALLER_ID, data) ;
            strName = data.getIdentity() ;
        } catch (PCallAddressException e) {
            SysLog.log(e) ;
        }

        return strName ;
    }

    /**
     * What is the display address of the SIP URL? The data returned here
     * depends on the SIP URL.  The SIP URL is returned minus any tags
     * or display names
     */
    protected String getDisplayAddress(String strSIPURL)
    {
        String strAddress = strSIPURL ;

        SipParser parser = new SipParser(strSIPURL) ;

        try {
            SipParser builder = new SipParser() ;

            builder.setUser(parser.getUser()) ;
            builder.setHost(parser.getHost()) ;
            builder.setPort(parser.getPort()) ;

            strAddress = builder.render() ;
            strAddress.trim() ;
        } catch (Exception e) {
            strAddress = strSIPURL ;
        }
        return strAddress ;
    }
}