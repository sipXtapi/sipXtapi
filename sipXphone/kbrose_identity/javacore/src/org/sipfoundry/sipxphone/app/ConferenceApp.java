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

package org.sipfoundry.sipxphone.app ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.callcontrol.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.sys.app.ShellApp ;

import org.sipfoundry.sipxphone.app.conference.* ;


/**
 * 'The' Conference Application
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ConferenceApp extends Application
{
    protected  ConferenceStatusForm m_formConferenceStatus ;


    public void main(String argv[])
    {
        PCall callToConference = Shell.getCallManager().getInFocusCall() ;
        AppResourceManager.getInstance().addStringResourceFile("Conference.properties") ;

        // Create a call if we don't have one
        if (callToConference == null) {
            try {
                callToConference = PCall.createCall(PCall.CREATE_HINT_CONFERENCE) ;
                callToConference.playTone(PtTerminalConnection.DTMF_TONE_DIALTONE) ;
                try {
                    Shell.getMediaManager().setDefaultAudioDevice() ;
                } catch (PMediaLockedException e) { /* burp */ }
            } catch (PCallException e) {
                SysLog.log(e) ;
                
                MessageBox messageBox = new MessageBox(this, MessageBox.TYPE_ERROR) ;
                messageBox.setMessage(messageBox.getString("lblErrorMaxConnections")) ;
                messageBox.showModal() ;
            }
        }
        else
        {
            if (Shell.getXpressaSettings().getProductID() == XpressaSettings.PRODUCT_XPRESSA)
            {
                PtCallControlCall callJTAPI = (PtCallControlCall) callToConference.getJTAPICall() ;
                callJTAPI.setCodecCPULimit(PtCallControlCall.CODEC_CPU_LOW, true);
            }
        }

        if (callToConference != null) {
            // unhook the call from the core world
            Shell.getCallManager().ignoreCall(callToConference) ;

            m_formConferenceStatus = new ConferenceStatusForm(this, callToConference) ;
            Shell.getInstance().setDialingStrategy(new icConferenceDialerStrategy(m_formConferenceStatus), callToConference) ;

            // Display the Conference Form
            m_formConferenceStatus.showModal() ;
            Shell.getInstance().setDialingStrategy(null) ;
            m_formConferenceStatus = null ;
        }
        //remove the resource bundle
        AppResourceManager.getInstance()
            .removeStringResourceFile("Conference.properties") ;
    }

    public Application getApp()
    {
        return this ;
    }


    /**
     * Dialer Strategy used when attemping to add a conference participant a call
     */
    private class icConferenceDialerStrategy extends AbstractDialingStrategy
    {
        public icConferenceDialerStrategy(DialingStrategyListener listener)
        {
            super() ;

            if (listener != null)
                addDialingStrategyListener(listener) ;
        }


        /**
         * 'Dial' or invoke the method
         */
        public void dial(PAddress address)
        {
            PCall call = Shell.getInstance().getDialingStrategyCall() ;

            dial(address, call) ;
        }


        /**
         * Add the conference participant
         */
        public void dial(PAddress address, PCall call)
        {
            if (call != null) {
                try {
                    fireDialingInitiated(call ,address) ;
                } catch (Exception e) {
                    SysLog.log(e) ;                    
                }

                if (m_formConferenceStatus != null) {
                    // Make sure the address isn't already in our conference
                    if (m_formConferenceStatus.isActiveParticipant(address)) {
                        MessageBox messageBox = new MessageBox(getApp(), MessageBox.TYPE_ERROR) ;
                        messageBox.setMessage(messageBox.getString("lblConferenceAlreadyAdded")) ;
                        messageBox.showModal() ;
                    } else {
                        // Remove if it is unactive address
                        m_formConferenceStatus.removeInactiveParticipant(address) ;
                        try {
                            call.connect(address) ;
                        } catch (PCallResourceUnavailableException e) {
                            MessageBox messageBox = new MessageBox(getApp(), MessageBox.TYPE_ERROR) ;
                            messageBox.setMessage(messageBox.getString("lblErrorMaxConnections")) ;
                            messageBox.showModal() ;
                        } catch (PCallAddressException cae) {
                            MessageBox messageBox = new MessageBox(getApp(), MessageBox.TYPE_ERROR) ;
                            messageBox.setMessage(cae.getMessage()) ;
                            messageBox.showModal() ;
                        } catch (Exception e) {
                            MessageBox messageBox = new MessageBox(getApp(), MessageBox.TYPE_ERROR) ;
                            messageBox.setMessage(messageBox.getString("lblErrorTextUnhandledException") + e.getMessage()) ;
                            messageBox.showModal() ;
                        }

                    }
                }
            } else
                throw new IllegalArgumentException("Cannot connference a null call") ;
        }


        /**
         * Gets the string based representation of this strategy.  This text is
         * typically prepended the the form name.
         */
        public String getFunction()
        {
            return AppResourceManager.getInstance().getString("lblConferenceStategyFunction") ;
        }


        /**
         * Gets the string based representation of the dialing strategy action.
         * This text is typically displayed as the B3 button label
         */
        public String getAction()
        {
            return AppResourceManager.getInstance().getString("lblConferenceStategyAction") ;
        }

        /**
         * Get the string based instructions for this strategy.  This text is
         * typically displayed in lower half of the "dialer" form.
         */
        public String getInstructions()
        {
            return AppResourceManager.getInstance().getString("lblConferenceInstructions") ;
        }



        /**
         * Gets the hint text associatd with the dialing strategy action.  This
         * hint text is typically displayed when the B3 buttons is pressed and
         * held down.
         */
        public String getActionHint()
        {
            return AppResourceManager.getInstance().getString("hint/core/dial_strategy/conference") ;
        }


        /**
         * Get the cancel status for this dialing strategy.
         *
         * @return boolean true if this operation/strategy can be canceled,
         *         otherwise false
         */
        public boolean isCancelable()
        {
            return true ;
        }
    }
}
