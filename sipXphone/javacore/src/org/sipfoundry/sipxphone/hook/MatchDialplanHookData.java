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
 * Data object for a MatchDialplanHook. A MatchDialplanHook allows you 
 * to examine a dialing string and determine when a dialing plan has been
 * matched. Once a dialing plan is matched, digit collection is completed
 * and a call/connection is attempted.
 *
 * @see org.sipfoundry.sipxphone.hook.Hook
 * @see org.sipfoundry.sipxphone.sys.HookManager
 *  
 * @author Robert J. Andreasen, Jr.
 */
public class MatchDialplanHookData extends HookData
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    /** Does not match any known dial plan. */
    public static final int MATCH_FAILURE = 0 ;
    /** Matches a known dial plan. */
    public static final int MATCH_SUCCESS = 1 ;
    /** Matches a known dial plan; however, a timeout is requested (to collect
        further digits) before dialing. */
    public static final int MATCH_TIMEOUT = 2 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** Dial string in question. */
    private String m_strDialString ;
    /** Does the dial string match a dialing plan? */
    private int m_iMatchState ;
    /** Dial plan address. */
    private String m_strAddress ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Create a Match Dialplan hook data object with the specified dial
     * string.
     *
     * @param strDialString The dial string that will be evaluated by the Match
     *        Dialplan hook chain.
     *
     */
    public MatchDialplanHookData(String strDialString)
    {
        m_strDialString = strDialString ;
        m_iMatchState = MATCH_FAILURE ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Set the dial plan match state for dial string. Developers should only
     * call this method if they can positively confirm or deny the dial string.
     * If no action is taken, the next hook in the chain will evaluate the dial
     * string.
     *
     * @param iState One of the MATCH_* state constants described above.
     */
    public void setMatchState(int iState)
    {
        m_iMatchState = iState ;
        terminate() ;
    }


    /**
     * Set the dial plan match state and address for the dial string.
     * Developers should only call this method if they can positively confirm
     * or deny the dial string. If no action is taken, the next hook in the
     * chain will evaluate the dial string.
     *
     * @param iState One of the MATCH_* state constants described above.
     * @param strAddress The fully qualified address of dial string.
     */
    public void setMatchState(int iState, String strAddress)
    {
        m_strAddress = strAddress ;
        m_iMatchState = iState ;
        terminate() ;
    }


    /**
     * Get the dial plan match state for this dial string.
     *
     * @return Dial plan state constants; see the MATCH_* state constants 
     * defined above.
     */
    public int getMatchState()
    {
        return m_iMatchState ;
    }


    /**
     * Get the address of the dial string as specified in the 
     * <i>setMatchState</i> method.
     *
     * @return The fully qualified address of the dial string.
     */
    public String getAddress()
    {
        return m_strAddress ;
    }


    /**
     * Get the dial string that this hook is trying to evaluate.
     *
     * @return The dialing string candidate.
     */
    public String getDialString()
    {
        return m_strDialString ;
    }
}
