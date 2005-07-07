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

import java.util.Vector ;
import java.util.Enumeration ;
import javax.telephony.* ;

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.stapi.* ;
import org.sipfoundry.telephony.* ;

import org.sipfoundry.sipxphone.sys.app.* ;

import org.sipfoundry.sipxphone.awt.form.* ;


/**
 * This class exists purely to monitor/manage state when dialing the
 * phone.  It maintains the dial string and deals with the mechansims
 * to handle call plans.
 * <br><br>
 * This should probably have a custom listener/event pairs (or multiple),
 * but for now it fires off action events with params.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class DialStateMonitor implements PButtonListener, PActionListener
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** fired whenever an dial string is changed (backspace, new digit, etc) */
    public static final String ACTION_DIAL_STRING_CHANGE   = "action_dial_string_change" ;

    /** fired whenever the cursor is changed (left, right, etc.) */
    public static final String ACTION_CURSOR_POS_CHANGE   = "action_cursor_pos_change" ;

    /** fired whenever we think the string is complete (timeout, meet dialplan
        requirements, user has hit the # key, etc */
    public static final String ACTION_DIAL_STRING_COMPLETE = "action_dial_string_complete" ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** our dial string */
    private String  m_strDial ;
    /** current cursor position */
    private int m_iCursorPosition ;
    /** object requesting notification of dial state changes */
    private PActionListener m_listener ;
    /** have we completed / sent our dial string? */
    private boolean m_bCompleted ;

    private PCall   m_call ;

    /** should we automatically check for dial completion and auto dial? */
    private boolean m_bAutoDialplanParsing ;

    private PingerConfig m_config = null ;

    /** dial state monitor listeners */
    private Vector    m_listeners ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * default contstructor
     */
    public DialStateMonitor()
    {
        m_strDial = new String() ;
        m_iCursorPosition = 0 ;
        m_listeners = null ;
        m_bCompleted = false ;
        m_bAutoDialplanParsing = true ;

        m_config = PingerConfig.getInstance() ;

        onClear() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Enable or Disable dial plan parsing.
     *
     * You may or maynot want to automatically dial a number once it matches
     * a dial plan.  By default, the application will wait a fixed amount of
     * time before auto dialing.
     *
     * @param bEnable true to enable dial plan parsing (auto-dial) or false
     *        to disable.
     */
    public void enableDialplanParsing(boolean bEnable)
    {
        m_bAutoDialplanParsing = bEnable ;
        if (bEnable == false) {
            Timer.getInstance().removeTimers(this) ;
        }
    }


    /**
     * user has hit the backspace buttons
     */
    public void onBackspace()
    {
        int iCursorPosition = m_iCursorPosition ;

        // Kill the evaluation timer if the user starts mucking with data
        Timer.getInstance().removeTimers(this) ;

        // Make sure we are within bounds
        if ((iCursorPosition > 0) && (iCursorPosition <= m_strDial.length())) {
            doBackspaceTone() ;
            onBackward() ;
            setDialString(m_strDial.substring(0, iCursorPosition-1) + m_strDial.substring(iCursorPosition)) ;
        }
    }


    /**
     * move the cursor one character to the left/backward
     */
    public void onBackward()
    {
        // Kill the evaluation timer if the user starts mucking with data
        Timer.getInstance().removeTimers(this) ;
        setCursorPosition(m_iCursorPosition-1) ;

    }


    /**
     * move the cursor one character to the right/forward
     */
    public void onForward()
    {
        // Kill the evaluation timer if the user starts mucking with data
        Timer.getInstance().removeTimers(this) ;
        setCursorPosition(m_iCursorPosition+1) ;
    }


    public void clearAutoDialTimer()
    {
        Timer.getInstance().removeTimers(this) ;
    }


    /**
     * clear the dial string
     */
    public void onClear()
    {
        setDialString("") ;
        m_bCompleted = false ;
        m_call = null ;

        // Kill the evaluation timer if the user starts mucking with data
        Timer.getInstance().removeTimers(this) ;
    }


    /**
     * Initialize our state monitor and snag a reference to whatever call/
     * terminal connection is active.  Perhaps this should be passed in?
     */
    public void initialize()
    {
        m_call = Shell.getCallManager().getInFocusCall() ;
    }


    /**
     * @return the dial string
     */
    public String getDialString()
    {
        return m_strDial ;
    }


    /**
     * add the listener to this state object
     *
     * @HACK We should not be using action listeners, but should create
     *       a customer listener interface for this...
     */
    public void addDialStateMonitorListener(PActionListener listener)
    {
        if (m_listeners == null)
            m_listeners = new Vector() ;

        if (!m_listeners.contains(listener))
            m_listeners.addElement(listener) ;
    }


    /**
     * remove a listener from this state object
     *
     * @HACK We should not be using action listeners, but should create
     *       a customer listener interface for this...
     */
    public void removeDialStateMonitorListener(PActionListener listener)
    {
        if (m_listeners != null) {
            m_listeners.removeElement(listener) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
    * sets the position of the cursor within the editor
    *
    * @param iPosition the new position of the Cursor
    */
    protected void setCursorPosition(int iPosition)
    {
        int iOldPosition = m_iCursorPosition ;

        m_iCursorPosition = iPosition ;

        // Clip along lower bound
        if (m_iCursorPosition < 0)
            m_iCursorPosition = 0 ;

        // Clip along upper bound
        if (m_iCursorPosition > m_strDial.length())
            m_iCursorPosition = m_strDial.length() ;

        // Only report if necessary
        if (m_iCursorPosition != iOldPosition) {
            fireCursorChange(m_iCursorPosition) ;
        }
    }


    /**
     * set the dial string for our state monitor
     */
    protected void setDialString(String strDial)
    {
        String strOld = m_strDial ;

        if (strDial == null) {
            throw new IllegalArgumentException("cannot set null dial string, please use double quotes") ;
        }

        m_strDial = strDial ;

        if (!m_strDial.equals(strOld)) {
            // Validate Cursor Position before firing
            setCursorPosition(m_iCursorPosition) ;

            fireDialStringChange(strDial) ;
        }
    }


    /**
     * fire a cursor change notification to our audience
     */
    protected void fireCursorChange(int iCursor)
    {
        if (m_listeners != null) {
            PActionEvent event = new PActionEvent(this, ACTION_CURSOR_POS_CHANGE) ;
            event.setIntParam(iCursor) ;

            Enumeration enum = m_listeners.elements() ;
            while (enum.hasMoreElements()) {
                PActionListener listener = (PActionListener) enum.nextElement() ;
                if (listener != null) {
                    try {
                        listener.actionEvent(event) ;
                    } catch (Throwable t) {
                        Shell.getInstance().showUnhandledException(t, true) ;
                    }
                }
            }
        }
    }


    /**
     * fire a dial string change notification to our audience
     */
    protected void fireDialStringChange(String strDial)
    {
        if (m_listeners != null) {
            PActionEvent event = new PActionEvent(this, ACTION_DIAL_STRING_CHANGE) ;
            event.setObjectParam(strDial) ;

            Enumeration enum = m_listeners.elements() ;
            while (enum.hasMoreElements()) {
                PActionListener listener = (PActionListener) enum.nextElement() ;
                if (listener != null) {
                    try {
                        listener.actionEvent(event) ;
                    } catch (Throwable t) {
                        Shell.getInstance().showUnhandledException(t, true) ;
                    }
                }
            }
        }
    }


    /**
     * fire a dial string complete notification to our audience
     */
    protected void fireDialStringComplete(String strDial)
    {
        if (m_listeners != null) {
            PActionEvent event = new PActionEvent(this, ACTION_DIAL_STRING_COMPLETE) ;
            event.setObjectParam(strDial) ;

            Enumeration enum = m_listeners.elements() ;
            while (enum.hasMoreElements()) {
                PActionListener listener = (PActionListener) enum.nextElement() ;
                if (listener != null) {
                    try {
                        listener.actionEvent(event) ;
                    } catch (Throwable t) {
                        Shell.getInstance().showUnhandledException(t, true) ;
                    }
                }
            }
        }
        m_bCompleted = true ;
    }


    /**
     * The specified button has been press downwards
     */
    public void buttonDown(PButtonEvent event)
    {
        // No need to continue if we have already completed our 'work'
        if (m_bCompleted) {
            return ;
        }

        switch (event.getButtonID()) {
            case PButtonEvent.BID_0:
            case PButtonEvent.BID_1:
            case PButtonEvent.BID_2:
            case PButtonEvent.BID_3:
            case PButtonEvent.BID_4:
            case PButtonEvent.BID_5:
            case PButtonEvent.BID_6:
            case PButtonEvent.BID_7:
            case PButtonEvent.BID_8:
            case PButtonEvent.BID_9:
            case PButtonEvent.BID_STAR:
                char ch = (char) event.getButtonID() ;

                setDialString(m_strDial.substring(0, m_iCursorPosition) + ch + m_strDial.substring(m_iCursorPosition)) ;
                onForward() ;
                event.consume() ;

                beginDTMFTone(event.getButtonID()) ;
                break ;
            case PButtonEvent.BID_POUND:
                // Send is processed on the button up
                event.consume() ;
                break ;
        }
    }


    /**
     * The specified button has been released
     */
    public void buttonUp(PButtonEvent event)
    {

        // No need to continue if we have already completed our 'work'
        if (m_bCompleted)
            return ;

        switch (event.getButtonID()) {
            case PButtonEvent.BID_0:
            case PButtonEvent.BID_1:
            case PButtonEvent.BID_2:
            case PButtonEvent.BID_3:
            case PButtonEvent.BID_4:
            case PButtonEvent.BID_5:
            case PButtonEvent.BID_6:
            case PButtonEvent.BID_7:
            case PButtonEvent.BID_8:
            case PButtonEvent.BID_9:
            case PButtonEvent.BID_STAR:
                endTone() ;

                // We start up the 'evaluation' timer.  This will allow us to
                // take a look at our data in XXXX ms so that we can determine
                // if the dial string matches some dial plan and we can auto
                // send it.

                // We will only do that iff we are not in the middle of the string
                // (i.e. editing)

                if (m_bAutoDialplanParsing && (m_iCursorPosition == m_strDial.length())) {
                    int iTimeout = Settings.getInt("DIALER_EVALUATE_TIMEOUT", 1500) ;
                    Timer.getInstance().resetTimer(iTimeout, this, m_strDial) ;
                }
                event.consume() ;
                break ;
            case PButtonEvent.BID_POUND:
                // 'Send' Key
                fireDialStringComplete(m_strDial) ;
                event.consume() ;
                break ;
        }
    }


    /**
     * The specified button is being held down
     */
    public void buttonRepeat(PButtonEvent event)
    {

    }


    /**
     * called by the timer service whenever we need to evaluate our dial string
     * and see if it meets a dial plan.
     */
    public void actionEvent(PActionEvent event)
    {
        if (event.getActionCommand().equals(Timer.ACTION_TIMER_FIRED)) {

            String strDial = (String) event.getObjectParam() ;
            if (matchesDialPlan(strDial)) {
                fireDialStringComplete(strDial) ;
            }
        }
    }


    /**
     * Does the passed dial string candidate match a know dialing plan?
     *
     * @return boolean true if a match is found; otherwise false
     */
    protected boolean matchesDialPlan(String strDial)
    {
        int iPlanLength = Settings.getInt("DIALER_PLAN_DIGITS", 4) ;
        boolean bRC = false ;

        // Grab our phoneset dial plan length
        if (m_config != null) {
            String strLength = (String) m_config.getValue("PHONESET_DIALPLAN_LENGTH") ;
            if (strLength != null) {
                try {
                    iPlanLength = Integer.parseInt(strLength) ;
                } catch (Exception e) {
                    System.out.println("Invalid dial plan length: " + strLength) ;
                }
            }
        }

        if (strDial.length() >= iPlanLength) {
            fireDialStringComplete(strDial) ;
        }

        return bRC ;
    }


    protected void beginDTMFTone(int iButtonID)
    {
        // Obtain a reference to a terminal connection if needed
        if (m_call == null)
            initialize() ;

        if (m_call != null) {
            try {
                switch (iButtonID) {
                    case PButtonEvent.BID_0:
                        m_call.playTone(PCall.DTMF_0) ;
                        break ;
                    case PButtonEvent.BID_1:
                        m_call.playTone(PCall.DTMF_1) ;
                        break ;
                    case PButtonEvent.BID_2:
                        m_call.playTone(PCall.DTMF_2) ;
                        break ;
                    case PButtonEvent.BID_3:
                        m_call.playTone(PCall.DTMF_3) ;
                        break ;
                    case PButtonEvent.BID_4:
                        m_call.playTone(PCall.DTMF_4) ;
                        break ;
                    case PButtonEvent.BID_5:
                        m_call.playTone(PCall.DTMF_5) ;
                        break ;
                    case PButtonEvent.BID_6:
                        m_call.playTone(PCall.DTMF_6) ;
                        break ;
                    case PButtonEvent.BID_7:
                        m_call.playTone(PCall.DTMF_7) ;
                        break ;
                    case PButtonEvent.BID_8:
                        m_call.playTone(PCall.DTMF_8) ;
                        break ;
                    case PButtonEvent.BID_9:
                        m_call.playTone(PCall.DTMF_9) ;
                        break ;
                    case PButtonEvent.BID_STAR:
                        m_call.playTone(PCall.DTMF_STAR) ;
                        break ;

                }
            } catch (PCallException e) {
                SysLog.log(e) ;
            }
        } else {
            System.out.println("ERROR dialStateMonitor: null call") ;
        }
    }


    protected void endTone()
    {
        if (m_call != null) {
            try {
                m_call.stopTone() ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
        }
    }

    protected void doBackspaceTone()
    {
        // Obtain a reference to a terminal connection if needed
        if (m_call == null)
            initialize() ;

        if (m_call != null) {
            try {
                m_call.playTone(PtTerminalConnection.DTMF_TONE_BACKSPACE) ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
        }
    }
}
