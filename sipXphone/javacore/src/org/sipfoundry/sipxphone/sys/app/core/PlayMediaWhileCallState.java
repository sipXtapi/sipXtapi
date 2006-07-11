/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/core/PlayMediaWhileCallState.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys.app.core;

import javax.telephony.* ;
import javax.telephony.phone.* ;
import javax.telephony.callcontrol.* ;

//import java.util.* ;
import java.io.File;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.phone.* ;
import org.sipfoundry.telephony.phone.event.* ;
import org.sipfoundry.telephony.callcontrol.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.stapi.event.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.app.* ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.* ;
import org.sipfoundry.sipxphone.sys.app.core.* ;
import org.sipfoundry.sipxphone.sys.app.shell.* ;
import org.sipfoundry.sipxphone.service.* ;

import org.sipfoundry.sipxphone.awt.*;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.app.adjustvolume.* ;

import org.sipfoundry.sip.SipLineManager ;
import org.sipfoundry.sip.SipLine ;

import org.sipfoundry.sipxphone.sys.startup.PingerApp ;

public class PlayMediaWhileCallState {

    private icCommandDispatcher m_timerListener = new icCommandDispatcher();
    private PCall m_call = null;
    private int m_Tone = 0;
    private int m_state = 0;
    private int m_PlayDuration = 0;

    /**
     * This call will start a timer if a call state is active.
     * It will then monitor the call, and at defined intervals, will
     * play a tone if the state continues to be the state for which the object
     * is looking for
     *
     * @since 2.0.0
     *
     * @param call call to monitor for state
     * @param media media constant (See PtTerminalConnection for constants)
     * @param state Call state to monitor
     * @param intervalBetweenPlaysInMillisecs What delay before starting play
     * @param playDurationInMillisecs How long to play
     *
     * @return description
     * @exception description
     * @author First Last
     */

    public PlayMediaWhileCallState(PCall call,
                    int media,
                    int state,
                    int intervalBetweenPlaysInMillisecs,
                    int playDurationInMillisecs) {


        if (call != null && call.getConnectionState() == state)
        {
            //start the timer which fires events to play a  tone when phone is off  hook
            Timer timer = Timer.getInstance();

            //now add the timer
            timer.addTimer(intervalBetweenPlaysInMillisecs, m_timerListener, null, true);

            //save off some stuff for later.
            m_Tone  = media;
            m_call  = call;
            m_state = state;
            m_PlayDuration = playDurationInMillisecs;
        }
    }



//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////
    public class icCommandDispatcher implements PActionListener
    {
        private void alertCall()
        {
            try
            {
                m_call.stopTone() ;
                m_call.stopSound() ;
                m_call.playTone(m_Tone, m_PlayDuration, true, false) ;
            }
            catch (PSTAPIException e)
            {
                // ::TODO:: Handle this failure condition
                SysLog.log(e) ;
            }
        }
        /**
         * function that plays the off hook tone
         *
         * @since 2.0.0
         *
         *
         * @param event incoming event
         *
         * @return void
         * @author D. Winsor
         */
        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(Timer.ACTION_TIMER_FIRED))
            {
                // we got here because a timer was created when a call was placed on hold
                if (m_call == null || (m_call != null && m_call.getConnectionState() != m_state))
                {
                     Timer timer = Timer.getInstance();
                    timer.removeTimers(m_timerListener);
                }
                else
                    alertCall();
            }
        }

    }

}