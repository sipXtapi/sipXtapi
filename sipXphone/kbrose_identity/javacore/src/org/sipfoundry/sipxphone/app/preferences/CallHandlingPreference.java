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


package org.sipfoundry.sipxphone.app.preferences ;

import javax.telephony.* ;
import javax.telephony.callcontrol.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.callcontrol.* ;


/**
 * The CallHandlingPreference object is responsible for encapsulating
 * a single type of call forwarding.
 */
public class CallHandlingPreference
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Types of forwarding available */
    private static final int m_iPrefs[] = {CallControlForwarding.ALL_CALLS,
                                         CallControlForwarding.FORWARD_ON_BUSY,
                                         CallControlForwarding.FORWARD_ON_NOANSWER } ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    /** JTAPI forwarding instructions */
    private CallControlForwarding[] m_callControlForwarding ;
    /** Reference to our PtCallControlAddress object - CallControlForwarding objecting is dependent on this */
    private PtCallControlAddress m_callAddress ;
    /** Is this Preference forwarded ? */
    private boolean m_bIsForwarded ;
    /** Forwarding address */
    private String m_strAddress ;
    /** Type of forwarding, as defined in CallControlForwarding */
    private int m_iForwardingType ;

    public CallHandlingPreference(int iType)
    {
        // Set the type of forwarding instruction
        m_iForwardingType = iType ;
        m_strAddress = "" ;
        m_bIsForwarded = false ;

        // Get the current address from the shell
        // Note currently have only 1 address associated with a phone
        Address addresses[] = ShellApp.getInstance().getTerminal().getAddresses() ;
        m_callAddress = (PtCallControlAddress) addresses[0];

        boolean m_bIsForwarded = false ;

        /** Find whether our preference is set in PtCallControlAddress */
        try {
            // Get our JTAPI Call Forwarding settings
            m_callControlForwarding = m_callAddress.getForwarding() ;

            for (int i=0 ; i< m_callControlForwarding.length; i++) {
                if (m_callControlForwarding[i] != null) {
                    if (m_callControlForwarding[i].getType() == m_iForwardingType) {
                        // If found, set internal data
                        m_strAddress = m_callControlForwarding[i].getDestinationAddress() ;
                        m_bIsForwarded = true ;
                    }
                }
            }
        } catch (MethodNotSupportedException e) {
            System.out.println("CallHandlingManager:: method not supported") ;
            SysLog.log(e) ;
        } catch (Exception e) {
            System.out.println("CallHandlingManager:: exception caught") ;
            SysLog.log(e) ;
        }

        if (!m_bIsForwarded) {
            m_strAddress = "" ;
        }

    }

    public String getDestinationAddress()
    {
        return m_strAddress ;
    }

    public boolean getIsForwarded()
    {
        return m_bIsForwarded ;
    }


    /**
     * Have to create a CallControlForwarding[] array, and pass it back to PTCallControlAddress.
     * If the forwarding preference is already set in the array, remove it and add new setting.
     *
     * @param sAddress New destination address for particular Call Control preference
     */
    public void setDestinationAddress(String strAddress)
    {
        CallControlForwarding callNewForwarding[] ;

        // Create new CallControlForwarding array
        if (m_strAddress == "")
            callNewForwarding = new CallControlForwarding[m_callControlForwarding.length + 1];
        else
            callNewForwarding = new CallControlForwarding[m_callControlForwarding.length];

        boolean m_bChanged = false ;
        int i = 0 ;
        try {
            for (i=0 ; i< m_callControlForwarding.length; i++) {
                if (m_callControlForwarding[i].getType() == m_iForwardingType) {
                    // Add new forwarding object in new array - don't copy over old value
                    callNewForwarding[i] = new CallControlForwarding(strAddress,m_iForwardingType) ;
                    m_bChanged = true ;
                } else {
                    // Copy forwarding object into new array
                    callNewForwarding[i] = new CallControlForwarding(m_callControlForwarding[i].getDestinationAddress(), m_callControlForwarding[i].getType()) ;
                }
            }
            if (!m_bChanged) {
                // Didn't find forwarding object in array, so must add new object
                callNewForwarding[i] = new CallControlForwarding(strAddress,m_iForwardingType, false) ;

            }

            // Set the new CallControlForwarding object
            m_callAddress.setForwarding(callNewForwarding) ;

        } catch (Exception e) {
            System.out.println("CallHandlingPreferences:: method not supported") ;
            SysLog.log(e) ;
        } 
        
        // Save entry to relevant Pinger-config value                
        switch(m_iForwardingType) {
            case CallControlForwarding.FORWARD_UNCONDITIONALLY:
                PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG, PingerConfig.SIP_FORWARD_UNCONDITIONAL, strAddress) ;
                break ;
            case CallControlForwarding.FORWARD_ON_BUSY :
                PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG, PingerConfig.SIP_FORWARD_ON_BUSY, strAddress) ;
                break ;
            case CallControlForwarding.FORWARD_ON_NOANSWER :
                PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG, PingerConfig.SIP_FORWARD_ON_NO_ANSWER, strAddress) ;
                break ;
        }

    }
}
