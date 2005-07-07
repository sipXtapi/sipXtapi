/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/MessageWaitingIndicatorManager.java#2 $
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

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.featureindicator.* ;
import org.sipfoundry.util.SysLog;

import java.util.* ;
import javax.telephony.* ;
import javax.telephony.phone.* ;
import javax.telephony.callcontrol.* ;

import org.sipfoundry.stapi.* ;
import org.sipfoundry.telephony.* ;


/**
 * This is a very simple manager that can handle multiple requests for
 * the voicemail indicator.  The last person to ask wins, however, once
 * that requestor turns off the indicator, someone else is given control
 * in a LIFO manor.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class MessageWaitingIndicatorManager
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Vector m_vIndicatorStack ;
    private int m_iLastSetState = PhoneLamp.LAMPMODE_OFF ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default Constructor
     */
    public MessageWaitingIndicatorManager()
    {
        m_vIndicatorStack = new Vector() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Set the desired message indicator state
     */
    public synchronized void setState(int iState, String strIdentifier)
    {
        int iIndex = findIdentifier(strIdentifier) ;
        if (iIndex == -1) {
            // Not found; add
            if (iState != PhoneLamp.LAMPMODE_OFF) {
                icIndicatorData data = new icIndicatorData(iState, strIdentifier) ;
                m_vIndicatorStack.addElement(data) ;
            }
        } else {
            icIndicatorData data = (icIndicatorData) m_vIndicatorStack.elementAt(iIndex) ;
            // Found figure out what we should do
            if (iState == PhoneLamp.LAMPMODE_OFF) {
                m_vIndicatorStack.removeElementAt(iIndex) ;
            } else {
                data.setState(iState) ;
                m_vIndicatorStack.setElementAt(data, iIndex) ;
            }
        }

        // Set the final state
        int iSetState = PhoneLamp.LAMPMODE_OFF ;
        if (m_vIndicatorStack.size() > 0) {
            icIndicatorData data = (icIndicatorData ) m_vIndicatorStack.lastElement() ;
            iSetState = data.getState() ;
        }
        setIndicatorState(iSetState) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Find the specified indentifier in our stack and return its index
     * position
     *
     * @return index position of the strIdentifier or -1 if not found
     */
    protected int findIdentifier(String strIdentifier)
    {
        int iRC = -1 ;
        int iLength = m_vIndicatorStack.size() ;

        for (int i=0; i<iLength; i++) {
            icIndicatorData data = (icIndicatorData) m_vIndicatorStack.elementAt(i) ;
            if ((data != null) && (data.getIdentifier().equals(strIdentifier))) {
                iRC = i ;
                break ;
            }
        }

        return iRC ;
    }


    /**
     * Phyiscally go ahead and set the indicator state.  This routine checks
     * the previously know state and will not take unnecessary actions.  This
     * also means that code which sets the indicator directly will break this
     * manager.  Be warned!
     */
    protected void setIndicatorState(int iState)
    {
        if (m_iLastSetState != iState) {
            AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;

            PhoneLamp lamp = asc.getButtonLamp("VOICE_MAIL") ;
            if (lamp != null) {
                try {
                    lamp.setMode(iState) ;
                } catch (InvalidArgumentException e) {
                    SysLog.log(e) ;
                }
            }
            m_iLastSetState = iState ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Nested/Inner Classes
////
    /**
     * Simple data encapsulation object
     */
    private class icIndicatorData
    {
        private int    m_iState ;
        private String m_strIdentifier ;

        public icIndicatorData(int iState, String strIdentifier)
        {
            m_iState = iState ;
            m_strIdentifier = strIdentifier ;
        }


        public int getState()
        {
            return m_iState ;
        }

        public void setState(int iState)
        {
            m_iState = iState ;
        }


        public String getIdentifier()
        {
            return m_strIdentifier ;
        }
    }
}
