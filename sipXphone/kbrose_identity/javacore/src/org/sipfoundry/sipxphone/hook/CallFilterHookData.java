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

package org.sipfoundry.sipxphone.hook ;

import org.sipfoundry.stapi.* ;

/**
 * Data object for a CallFilter hook.  A CallFilter hook allows application
 * developers to filter inbound calls before the user is alerted,
 * for example, before the phone starts to ring.  <p> When
 * you add a CallFilter hook in the Hook Manager, all hooks are processed
 * in the order they were added until a filter takes an explicit
 * action.  If no actions are taken, then the
 * DefaultCallFilterHook will <i>accept</i> the call.
 *
 * @author Robert J. Andreasen, Jr. *
 */
public class CallFilterHookData extends HookData
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** <i>getAction</i> constant: Accept call. */
    public static final int ACTION_ACCEPT   = 0 ;
    /** <i>getAction</i> constant: Redirect call. */
    public static final int ACTION_REDIRECT = 1 ;
    /** <i>getAction</i> constant: Decline call. */
    public static final int ACTION_DECLINE  = 2 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** address the hookers get to twiddle about */
    private PAddress m_address ;
    /** address that was called */
    private PAddress m_addressCalled ;
    /** what should we do? */
    private int m_iAction = ACTION_ACCEPT ;
    /** redirect address if hookers decide that */
    private PAddress m_redirectAddress ;
    /** call id of the incoming call */
    private String m_strCallID ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor, takes the inbound caller address.
     *
     * @param address Address of inbound caller.
     * @param callid of the incoming call
     */
    public CallFilterHookData(PAddress addressCalling, PAddress addressCalled, String strCallID)
    {
        m_address = addressCalling ;
        m_addressCalled = addressCalled ;
        m_strCallID = strCallID ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Redirect the call to the specified address.
     *
     * @param address Address to which the call should be redirected.
     *
     * <p>
     * NOTE: <i>redirect</i> causes an implicit <i>terminate</i>. No other
     * call filter hooks will be executed for this call.
     */
    public void redirect(PAddress address)
    {
        m_iAction = ACTION_REDIRECT ;
        m_redirectAddress = address ;
        terminate() ;
    }


    /**
     * Decline the inbound call.  The call will be treated as if the phone was
     * busy or in use.
     *
     * <p>
     * NOTE: <i>decline</i> causes an implicit <i>terminate</i>. No other
     * call filter hooks will be executed for this call.
     */
    public void decline()
    {
        m_iAction = ACTION_DECLINE ;
        terminate() ;
    }


    /**
     * Accept the inbound call.  The user will be alerted of the inbound call.
     *
     * <p>
     * NOTE: <i>accept</i> causes an implicit <i>terminate</i>. No other
     * call filter hooks will be executed for this call.
     */
    public void accept()
    {
        m_iAction = ACTION_ACCEPT ;
        terminate() ;
    }


    /**
     * What is the address of the caller?
     */
    public PAddress getAddress()
    {
        return m_address ;
    }


    /**
     * Returns the address dialed by a caller to initiate a connection. Because an 
     * xpressa phone may have multiple lines, or identities, with different addresses 
     * that all resolve to the same physical phone, this
     * method allows you to determine which address was dialed by the caller.
     *
     * @return The address dialed or null if no inbound calls have
     *         been received.
     */
    public PAddress getCalledAddress()
    {
        return m_addressCalled ;
    }


    /**
     * What is the call id of the inbound call?
     *
     * @deprecated DO NOT EXPOSE
     */
    public String getCallID()
    {
        return m_strCallID ;
    }


    /**
     * What action was finally selected?
     *
     * @return ACTION_ACCEPT, ACTION_DECLINE, or ACTION_REDIRECT constant.
     */
    public int getAction()
    {
        return m_iAction ;
    }


    /**
     * What is the redirect address if we are to redirect this call?
     *
     * @param redirect Address, or null if not valid.
     */
    public PAddress getRedirectAddress()
    {
        return m_redirectAddress ;
    }
}
