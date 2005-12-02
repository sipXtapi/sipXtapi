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


package org.sipfoundry.sipxphone.sys ;

import javax.telephony.* ;
import javax.telephony.callcontrol.* ;
import javax.telephony.phone.* ;


import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.service.* ;

import org.sipfoundry.sipxphone.awt.event.* ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.phone.* ;
import org.sipfoundry.telephony.phone.event.* ;

import org.sipfoundry.util.PingerConfig;


/**
 * This class adds a level of abstraction to the shell/core applications.  It
 * feeds of all of the event streams within the system, managages states, and
 * then fires tailored events to shell/core application.
 * <br><br>
 * The goals being:
 * <ul>
 * <li>events contained all information need for processing - no need to query/poll</li>
 * <li>no need for state management outside of this beast</li>
 * <li>convenience methods such as getActiveCall(), getCallsOnHold(), etc.</li>
 * </ul>
 * <br><br>
 *
 * ** THIS SOURCE FILE IS OBSOLETE, DO NOT USE / REFERENCE IT ** * *
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PhoneState
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** is the phone currently on or off hook */
    public boolean m_bOnHook = true ;

    /** is the phone in the headset acts as the hook switch mode? */
    protected boolean m_bHeadsetAsHandset ;

    /** single instance of self */
    static protected PhoneState m_instance ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * default no argument constructor
     */
    private PhoneState()
    {
        PingerConfig config = PingerConfig.getInstance() ;

        String strValue = config.getValue(PingerConfig.PHONESET_HOOKSWITCH_CONTROL) ;
        if ((strValue != null) && strValue.equalsIgnoreCase("HEADSET_BUTTON"))
        {
            m_bHeadsetAsHandset = true ;
        }
        else
        {
            m_bHeadsetAsHandset = false ;
        }
    }


    public static PhoneState getInstance()
    {
        if (m_instance == null)
            m_instance = new PhoneState() ;

        return m_instance ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Is the phone currently on hook?
     */
    public boolean isOnHook()
    {
        return m_bOnHook ;
    }


    /**
     * Is the phone working in the headset as handset mode?
     */
    public boolean isHeadsetAsHandset()
    {
        return m_bHeadsetAsHandset ;
    }


    /**
     * Set the handset in either the onhook state (true) or offhook state
     * (false).
     */
    public void setOnHook(boolean bOnHook)
    {
        m_bOnHook = bOnHook ;
    }
}
