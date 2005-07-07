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


package org.sipfoundry.sipxphone.awt ;

import java.awt.* ;
import java.awt.event.* ;
import java.util.Vector ;
import java.util.Enumeration ;

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.hook.* ;
import org.sipfoundry.util.SysLog;

import org.sipfoundry.sipxphone.sys.app.core.DialingTemplateMatch ;
import org.sipfoundry.sipxphone.sys.app.core.DialingTemplate ;

import org.sipfoundry.stapi.* ;

import org.sipfoundry.telephony.* ;

/*
 * Implementation Notes:
 *
 * When receiving digits, the PDialField makes use of two timeouts:
 *
 * 1) A short pre-match timeout called 'EVALUATE' is set after a character is
 *    received, but before we send the dial string out for evaluation.  This
 *    timeout allows the user to enter multiple keys before spending the CPU
 *    time to evaluate the timeout.  It also stops dial plans from prematurely
 *    short-circuiting.  (Consider .... vs 91..........)
 *
 * 2) A longer post-match timeout called 'MATCH_TIMEOUT' is set after a dial
 *    plan is matched with a MATCH_TIMEOUT state.  This allows the user a
 *    little extra time if entering a longer digit string (i.e. an
 *    international number).  This timeout is optional and must be
 *    specifically requested in the digit map.
 */


/**
 * The dial field control adds dial plan intelligence to a standard text
 * field.
 * @see org.sipfoundry.sipxphone.awt.PTextField
 */
public class PDialField extends PTextField
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final String ACTION_DIAL_STRING_COMPLETE = "action_dial_string_complete" ;

    protected final static String TIMER_EVALUATE = "EVALUATE" ;
    protected final static String TIMER_MATCH_TIMEOUT = "MATCH_TIMEOUT" ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** font used to when displaying text */
    protected Font      m_displayFont = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT) ;
    /** the dial state monitor tracks key presses and interprets/understands them */
    protected DialStateMonitor m_dialStateMonitor ;

    protected Vector m_vListeners = null ;

    private PCall m_call = null ;

    protected icTimerHandler m_timerHandler ;

    private String m_strMatchAddress ;

    private boolean m_bButtonDown ;

    private boolean m_bClearDialTone ;  // Clear dial tone on next button down

    private DialingTemplate m_template ; // Display Template


//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Construct a PDialField with an initially empty dial string.
     */
    public PDialField()
    {
        super() ;
        m_bButtonDown = false ;
        m_timerHandler = new icTimerHandler() ;
        setAlphanumericMode(false) ;
        addTextListener(new icDialFieldMonitor()) ;
        m_strMatchAddress = null ;

        m_template = new DialingTemplate() ;
    }


    /**
     * Construct a PDialField with a specified dial string.
     *
     * @param strDialString The dial string text to supply.
     */
    public PDialField(String strDialString)
    {
        super(strDialString) ;
        m_timerHandler = new icTimerHandler() ;
        setAlphanumericMode(false) ;
        addTextListener(new icDialFieldMonitor()) ;

        m_template = new DialingTemplate() ;
        evaluateDialingTemplate() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Stop sounding any DTMF tones.  This method will only stop DTMF tones
     * started by this component.
     */
    public void stopTones()
    {
        if (m_bButtonDown)
        {
            endTone() ;
        }
    }


    /**
     * Clears the dial field and associated call.
     */
    public void clear()
    {
        m_call = null ;
        setText("") ;
        m_strMatchAddress = null ;
    }

    public void setClearDialTone()
    {
        m_bClearDialTone = true;
    }


    /**
     * Clears any evaluation and auto-dialing timers.
     */
    public void clearTimers()
    {
        Timer.getInstance().removeTimers(m_timerHandler) ;
    }


    /**
     * Adds an action listener to this PDialField.  Listeners are notified
     * when a dial string is completed.
     *
     * @param listener The action listener that will receive dial string
     *        completion events in the form of action events.
     * @see org.sipfoundry.sipxphone.awt.event.PActionListener
     */
    public void addActionListener(PActionListener listener)
    {
        if (m_vListeners == null)
            m_vListeners = new Vector() ;

        if (!m_vListeners.contains(listener))
            m_vListeners.addElement(listener) ;
    }


    /**
     * Removes the specified action listener from this PDialField.
     *
     * @param listener The PActionListener that will cease to receive dial
     *        string completion events in the form of action events.
     * @see org.sipfoundry.sipxphone.awt.event.PActionListener
     */
    public void removeActionListener(PActionListener listener)
    {
        if (m_vListeners != null) {
            m_vListeners.removeElement(listener) ;
        }
    }


    /**
     * Backspaces one character from the current caret position.
     */
    public void backspace()
    {
        doBackspaceTone() ;
        super.backspace() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Invoked whenever a button is depressed
     */
    protected void onButtonDown(PButtonEvent event)
    {

        m_bButtonDown = true ;

        if (event.getEventType() == event.KEY_DOWN)
        {
            if (event.getButtonID() == 8) //backspace
            {
                backspace();
                m_bButtonDown = false ;
                event.consume();
            }
            else if (event.getButtonID() == PButtonEvent.BID_HOME) //HOME KEY
            {
                super.Home();
                super.onButtonUp(event) ;
                m_bButtonDown = false ;
                event.consume();
            }
            else if (event.getButtonID() == PButtonEvent.BID_END) //END KEY
            {
                super.End();
                super.onButtonUp(event) ;
                m_bButtonDown = false ;
                event.consume();
            }
            // copy or paste
            else if (event.getButtonID() == CTRL_V || event.getButtonID() == CTRL_C)
            {
                super.onButtonDown(event);
                return;
            }
            else
            {
                if (!Character.isDigit((char)event.getButtonID()) &&
                    (char)event.getButtonID() != '*' &&
                    (char)event.getButtonID() != '#'
//DWW                    (char)event.getButtonID() != '.'
                    )
                {
                    event.consume();
                    return; //dont process letters on a non alph text field
                }


                beginDTMFTone(event.getButtonID()) ;
                super.insertCharacter((char)event.getButtonID()) ;
                event.consume();
            }
        }
        else
        {
            switch (event.getButtonID())
            {
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


                    beginDTMFTone(event.getButtonID()) ;
                    super.onButtonDown(event) ;
                    break ;
                case PButtonEvent.BID_POUND:
                    // Send is processed on the button up
                    event.consume() ;
                    break ;
            }
        }
    }


    /**
     * Invoked whenever a button is released
     */
    protected void onButtonUp(PButtonEvent event)
    {
        m_bButtonDown = false ;

        switch (event.getButtonID())
        {
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

                super.onButtonUp(event) ;
                break ;
            case PButtonEvent.BID_POUND:
                // 'Send' Key
                fireDialplanCompletion(getText()) ;
                event.consume() ;
                break ;
        }
    }

    private void beginDTMFTone(int iButtonID, boolean bLocal, boolean bRemote)
    {
        // Obtains a reference to a terminal connection if needed
        if (m_call == null)
            initialize() ;
 //       System.out.println("PDialField::beginDTMFTone  iButton = " + iButtonID);
        if (m_call != null) {

            //this basically is here to stop the DIALTONE if playing.
            if (m_bClearDialTone)
            {
                try {
                    m_call.stopTone() ;
                } catch (PSTAPIException e) {
                    SysLog.log(e) ;
                }
                m_bClearDialTone = false;
            }

            try {
                switch (iButtonID) {
                    case PButtonEvent.BID_0:
                        m_call.playTone(PCall.DTMF_0, bLocal, bRemote) ;
                        break ;
                    case PButtonEvent.BID_1:
                        m_call.playTone(PCall.DTMF_1, bLocal, bRemote) ;
                        break ;
                    case PButtonEvent.BID_2:
                        m_call.playTone(PCall.DTMF_2, bLocal, bRemote) ;
                        break ;
                    case PButtonEvent.BID_3:
                        m_call.playTone(PCall.DTMF_3, bLocal, bRemote) ;
                        break ;
                    case PButtonEvent.BID_4:
                        m_call.playTone(PCall.DTMF_4, bLocal, bRemote) ;
                        break ;
                    case PButtonEvent.BID_5:
                        m_call.playTone(PCall.DTMF_5, bLocal, bRemote) ;
                        break ;
                    case PButtonEvent.BID_6:
                        m_call.playTone(PCall.DTMF_6, bLocal, bRemote) ;
                        break ;
                    case PButtonEvent.BID_7:
                        m_call.playTone(PCall.DTMF_7, bLocal, bRemote) ;
                        break ;
                    case PButtonEvent.BID_8:
                        m_call.playTone(PCall.DTMF_8, bLocal, bRemote) ;
                        break ;
                    case PButtonEvent.BID_9:
                        m_call.playTone(PCall.DTMF_9, bLocal, bRemote) ;
                        break ;
                    case PButtonEvent.BID_STAR:
                        m_call.playTone(PCall.DTMF_STAR, bLocal, bRemote) ;
                        break ;
                }
            }
            catch (PSTAPIException e)
            {
                SysLog.log(e) ;
            }
        }
        else
        {
            System.out.println("ERROR dialStateMonitor: null call") ;
        }
    }


    /**
     * Begins to play the specified DTMF tone.
     */
    private void beginDTMFTone(int iButtonID)
    {
        beginDTMFTone( iButtonID, true, false );
    }


    /**
     * Ends/stops and playing tones
     */
    private void endTone()
    {
        if (m_call == null)
            initialize() ;

        if (m_call != null) {
            try {
                m_call.stopTone() ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
        }
    }


    /**
     * Plays a backspace tone
     */
    private void doBackspaceTone()
    {
        // Obtain a reference to a terminal connection if needed
        if (m_call == null)
            initialize() ;

        if (m_call != null) {
            try {
//                m_call.playTone(PtTerminalConnection.DTMF_TONE_BACKSPACE,250) ;
                m_call.playTone(PtTerminalConnection.DTMF_TONE_BACKSPACE, 250,true, false) ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
        }
    }


    /**
     * Initializes our state monitor and snag a reference to whatever call/
     * terminal connection is active.  Perhaps this should be passed in?
     */
    private void initialize()
    {
        m_call = Shell.getCallManager().getInFocusCall() ;
    }


    /**
     * fires off a dial plan completion event to all interested action listeners
     */
    private void fireDialplanCompletion(String strDialString)
    {
        if (m_vListeners != null) {
            Enumeration enum = m_vListeners.elements() ;
            PActionEvent event = new PActionEvent(this, ACTION_DIAL_STRING_COMPLETE) ;
            event.setObjectParam(strDialString) ;
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


    protected void evaluateDialingTemplate()
    {
/*
        DialingTemplateMatch match = m_template.apply(getText()) ;
        if (match != null)
        {
            String strRendering = match.getRenderedString() ;
            System.out.println("Dialing String: " + strRendering) ;
        }
*/
    }


    /**
     * Evaluate the current dial string for dial plan completion.  Also,
     * check against display templates
     */
    public int evaluateDialString()
    {
        HookManager hookManager = Shell.getHookManager() ;
        int         iRC = MatchDialplanHookData.MATCH_FAILURE ;
        boolean     bComplete = false ;

        MatchDialplanHookData data = new MatchDialplanHookData(getText()) ;
        hookManager.executeHook(HookManager.HOOK_MATCH_DIALPLAN, data) ;

        iRC = data.getMatchState() ;
        switch (iRC) {
            case MatchDialplanHookData.MATCH_FAILURE:
                break ;
            case MatchDialplanHookData.MATCH_SUCCESS:
                fireDialplanCompletion(getText()) ;
                bComplete = true ;
                break ;
            case MatchDialplanHookData.MATCH_TIMEOUT:
                int iTimeout = Settings.getInt("DIALER_MATCH_TIMEOUT", 1500) ;
                Timer.getInstance().resetTimer(iTimeout, m_timerHandler, TIMER_MATCH_TIMEOUT) ;
                break ;
        }

        // Check to see if we match a dialplan template
        if (bComplete == false)
        {
            evaluateDialingTemplate() ;
        }


        return iRC ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////
    /**
     * The dial field monitor listens for text value and caret position
     * changes events and sets/clears the evaluation timer accordingly.
     */
    private class icDialFieldMonitor implements PTextListener
    {
        /**
         * Invoked when the text field's data has changed
         */
        public void textValueChanged(PTextEvent event)
        {
            String strText = getText() ;
            int    iCaretPosition = getCaretPosition() ;

            // Set/Clear Evalulation Timer
            if ((strText != null) && (strText.length() == iCaretPosition)) {
                int iTimeout = Settings.getInt("DIALER_EVALUATE_TIMEOUT", 1500) ;
                Timer.getInstance().resetTimer(iTimeout, m_timerHandler, PDialField.TIMER_EVALUATE) ;
            } else
                Timer.getInstance().removeTimers(m_timerHandler) ;
        }


        /**
         * Invoked when the text field's caret position has changed
         */
        public void caretPositionChanged(PTextEvent event)
        {
            String strText = getText() ;
            int    iCaretPosition = getCaretPosition() ;

            // Set/Clear Evalulation Timer
            if ((strText != null) && (strText.length() == iCaretPosition)) {
                int iTimeout = Settings.getInt("DIALER_EVALUATE_TIMEOUT", 1500) ;
                Timer.getInstance().resetTimer(iTimeout, m_timerHandler, PDialField.TIMER_EVALUATE) ;
            } else
                Timer.getInstance().removeTimers(m_timerHandler) ;
        }
    }


    /**
     * Listens for Timer Changes
     */
    private class icTimerHandler implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            if (m_bButtonDown == true)
            {
                Timer.getInstance().resetTimer(100, this, event.getObjectParam()) ;
            }
            else if (TIMER_EVALUATE.equals(event.getObjectParam()))
            {
                evaluateDialString() ;
            }
            else if (TIMER_MATCH_TIMEOUT.equals(event.getObjectParam()))
            {
                fireDialplanCompletion(getText()) ;
            }
        }
    }
}
