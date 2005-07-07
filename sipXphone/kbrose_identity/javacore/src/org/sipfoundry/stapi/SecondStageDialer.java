/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/stapi/SecondStageDialer.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.stapi;

import org.sipfoundry.stapi.* ;
import org.sipfoundry.stapi.event.* ;
import org.sipfoundry.util.PhoneNumberParser ;
import org.sipfoundry.util.SysLog ;
import org.sipfoundry.sipxphone.sys.util.DTMFPlayer ;
import org.sipfoundry.sipxphone.sys.Shell ;

import org.sipfoundry.sipxphone.sys.app.ShellApp;
import org.sipfoundry.sipxphone.sys.app.shell.FormManager;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.sys.app.core.CallInProgressForm ;
import org.sipfoundry.sipxphone.awt.form.PAbstractForm;
import org.sipfoundry.sipxphone.awt.form.PForm;


/**
 * Class created with a call and PhoneNumberParser for the purpose of
 * completing second stage dialing, where second stage dialing includes waits
 * pauses and any other dialing strings.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SecondStageDialer extends Thread
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    protected static final int PAUSE_DELAY = 1000 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected PCall m_call ;
    protected PhoneNumberParser m_parser ;
    protected boolean m_bStarted ;
    protected boolean m_bExitImmediately ;
    protected boolean m_waiting ;
    protected Object m_waitlock = new Object();
    protected boolean m_bSilentDTMF ;

    protected icConnectionListener m_connectionListener ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public SecondStageDialer(PCall call, PhoneNumberParser parser)
    {
        m_call = call ;
        m_parser = parser ;
        m_bStarted = false ;
        m_bExitImmediately = false ;
        m_bSilentDTMF = false ;

        m_connectionListener = new icConnectionListener() ;

        call.addConnectionListener(m_connectionListener) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Main processing loop, walk through the various actions, processing
     * them sequentially.
     */
    public void run()
    {
        boolean bDone ;
        // Make sure we don't start TOO quickly, pause before starting to
        // process.  This allows the CPU to settle a bit after the initial
        // connection.
        pause() ;

        if (m_parser.getAction() != PhoneNumberParser.ACTION_INVALID)
        {
            // accumulate data first because when prompting user to wait
            // we tell them what digits are next
            int[] actions = new int[m_parser.countActions()];
            int[] actionInts = new int[actions.length];
            String[] actionStrings = new String[actions.length];
            for (int i = 0; i < actions.length; i++)
            {
                actions[i] = m_parser.getAction();
                actionInts[i] = m_parser.getActionInt();
                actionStrings[i] = m_parser.getActionString();
                m_parser.next();
            }

            for (int i = 0; i < actions.length && !m_bExitImmediately; i++)
            {
                switch (actions[i])
                {
                    case PhoneNumberParser.ACTION_WAIT:
                        //
                        // Wait for user reponse
                        //
                        String upcomingDtmf = buildUpcomingDtmf(i, actions, actionStrings);
                        if (!userWait(upcomingDtmf))
                            m_bExitImmediately = true;

                        break ;
                    case PhoneNumberParser.ACTION_PAUSE:
                        //
                        // Pause for a pre-determined amount of time
                        //
                        pause() ;
                        break ;
                    case PhoneNumberParser.ACTION_DIAL:
                        //
                        // Play a series of DTMF digits
                        //
                        playDTMF(actionStrings[i]) ;
                        break ;
                    case PhoneNumberParser.ACTION_SET_DEVICE:
                        //
                        // Enable the specified audio device
                        //
                        try
                        {
                            PMediaManager manager = Shell.getMediaManager() ;

                            //
                            // WAIT: Before you change the audio device, you must
                            // look at the enabled audio device.  The user may have
                            // picked up the handset or switched to speaker phone
                            // and activated the speaker.  So, we only want to
                            // change the audio device if it is as expected -- NONE
                            int iCurrentAudioID = manager.getAudioDeviceID() ;
                            if (iCurrentAudioID == PAudioDevice.ID_NONE)
                            {
                                manager.setAudioDevice(actionInts[i]) ;
                            }

                            // This is set by the RingDownProxyDialerStrategy.
                            m_call.removeCallData("AUTO_ENABLE_DEVICE") ;
                        }
                        catch (Exception e)
                        {
                            SysLog.log(e);
                        }
                        break ;
                }
            }
        }


        // Don't touch the call if we have been asked to exit immediately.
        // This object (and the call) will go out of scope soon.
        if (!m_bExitImmediately)
            m_call.removeConnectionListener(m_connectionListener) ;

        Shell.getInstance().clearStatus() ;
    }

    /**
     *
     */
    private String buildUpcomingDtmf(int index, int[] actions, String[] actionStrings)
    {
        StringBuffer sb = new StringBuffer();
        boolean done = false;
        for (int i = index + 1; i < actions.length && !done; i++)
        {
            if (actions[i] == PhoneNumberParser.ACTION_DIAL)
                sb.append(actionStrings[i]);
            else if (actions[i] == PhoneNumberParser.ACTION_PAUSE)
                sb.append(PhoneNumberParser.CHAR_PAUSE);
            else
                break;
        }

        return sb.toString();
    }






    /**
     * Controls whether DTMF is played to the local speaker.  DTMF is always
     * played remotely.
     *
     * @param bSilent true to only play remotely, false to play both
     *        remotely
     */
    public void setSilentDTMF(boolean bSilent)
    {
        m_bSilentDTMF = bSilent ;
    }




//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Play the series of DTMF tones being mindful that it may need to abort
     * if m_bExitImmediately is set.
     */
    protected void playDTMF(String strDTMF)
    {

        for (int i=0; i<strDTMF.length(); i++)
        {
            DTMFPlayer.playDTMFTone(m_call, strDTMF.charAt(i), !m_bSilentDTMF, true) ;
            if (m_bExitImmediately)
                break ;
        }
    }

    /**
     * Wait for user before proceeding
     */
    protected boolean userWait(String digits)
    {
        FormManager forms = ShellApp.getInstance().getFormManager();
        PForm form = forms.getActiveForm();
        if (form instanceof CallInProgressForm)
        {
            CallInProgressForm cip = (CallInProgressForm)form;
            return cip.promptToSendDtmf(digits);
        }

        return true;
    }



    /**
     * Pause for a pre-defined period of time
     */
    protected void pause()
    {
        try
        {
            Thread.sleep(PAUSE_DELAY) ;
        }
        catch (InterruptedException ie)
        {
            // Nothing to do
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Nested Classes
////

    /**
     * Listen for connection events.  If any negative events are fired,
     * set the exit immediately flag.  Otherwise, once the call is connected,
     * start the processing thread.
     */
    protected class icConnectionListener extends PConnectionListenerAdapter
    {
        public void connectionConnected(PConnectionEvent event)
        {
            if (!m_bStarted)
            {
                m_bStarted = true ;
                start() ;
            }
        }


        public void callDestroyed(PConnectionEvent event)
        {
            m_bExitImmediately = true ;
        }


        public void connectionFailed(PConnectionEvent event)
        {
            m_bExitImmediately = true ;
        }


        public void connectionDisconnected(PConnectionEvent event)
        {
            m_bExitImmediately = true ;
        }
    }
}
