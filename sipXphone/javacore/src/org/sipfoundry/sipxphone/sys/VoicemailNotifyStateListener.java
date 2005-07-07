/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/VoicemailNotifyStateListener.java#2 $
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

import java.awt.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import javax.telephony.phone.PhoneLamp ;

import org.sipfoundry.stapi.* ;

import org.sipfoundry.sipxphone.featureindicator.* ;

/**
 * This voice mail state listener listens for reboot requests and for voice
 * mail updates.  This code assumes that the user is only provisioned for
 * one voice mail service and assumes that the service does not send both
 * binary and detailed notifications.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class VoicemailNotifyStateListener
{
    private static VoicemailNotifyStateListener m_instance ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private icBinaryMessageFeatureIndicator m_fiBinary ;
    private icDetailMessageFeatureIndicator m_fiDetail ;
    private boolean m_bShouldDisplayNoVoicemail ;
    private String  m_strVoicemailRetrieveURL ;
    private int m_iMWILampMode = Shell.getXpressaSettings().geMWILampMode();



//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default private constructor
     */
    private VoicemailNotifyStateListener()
    {
        m_fiBinary = new icBinaryMessageFeatureIndicator() ;
        m_fiDetail = new icDetailMessageFeatureIndicator() ;

        PingerConfig config = PingerConfig.getInstance() ;

        // Determine if we should display the voicemail indicator if no
        // messages are available.
        String strEmptyBehavior = config.getInstance().getValue(
                PingerConfig.PHONESET_VOICEMAIL_EMPTY_BEHAVIOR) ;
        if ((strEmptyBehavior != null) && (strEmptyBehavior.equalsIgnoreCase("HIDE")))
        {
            m_bShouldDisplayNoVoicemail = false ;
        }
        else
        {
            m_bShouldDisplayNoVoicemail = true ;
        }


        // Get the Voice Mail Server
        m_strVoicemailRetrieveURL = config.getValue(PingerConfig.PHONESET_VOICEMAIL_RETRIEVE) ;
        if ((m_strVoicemailRetrieveURL != null) && (m_strVoicemailRetrieveURL.length() == 0)) {
            m_strVoicemailRetrieveURL = null ;
        }

    }


    /**
     * Initialize the VoicemailNotifyStateListener by adding a feature
     * indicator if the we have a fetch URL and m_bShouldDisplayNoVoicemail
     * is enabled.
     */
    public void initialize()
    {
        if ((m_strVoicemailRetrieveURL != null) && m_bShouldDisplayNoVoicemail)
            m_fiBinary.install() ;

    }


    /**
     * Singleton accessor method
     */
    public static VoicemailNotifyStateListener getInstance(boolean bShouldCreate)
    {
        if (m_instance == null) {
            if (bShouldCreate)
                m_instance = new VoicemailNotifyStateListener() ;
            else
            {
                System.out.println("VoicemailNotifyStateListener getInstance called with false and it wasn't previously created!\n");
                m_instance = null;
            }
        }

        return m_instance ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Called when there is a change in voice mail status.  This is a simple
     * callback that will be used if/when the voice mail provider does not
     * support more detail.
     */
    public static void doBinaryMessageWaiting(String strURL,
            boolean bNewMessages)
    {
        //false so we always ask for the one which should be created. Complain if one is not created yet.
        VoicemailNotifyStateListener voicemailNotify = getInstance(false);
        if (voicemailNotify != null)
            voicemailNotify.updateBinaryMessageWaiting(strURL, bNewMessages) ;
        else
            System.out.println("VoicemailNotifyStateListener doBinaryMessageWaiting called before object initialized!\n");
    }


    /**
     * Called when there is a change in voice mail status.  This has various
     * bits of detail that may be either deltas or absolute values.
     */
    public static void doDetailMessageWaiting(String strURL,
            String  strMessageMediaType,
            boolean bAbsoluteValues,
            int     iTotalNewMessages,
            int     iTotalOldMessages,
            int     iTotalUntouchedMessages,
            int     iUrgentUntouchedMessages,
            int     iTotalSkippedMessages,
            int     iUrgentSkippedMessages,
            int     iTotalFlaggedMessages,
            int     iUrgentFlaggedMessages,
            int     iTotalReadMessages,
            int     iUrgentReadMessages,
            int     iTotalAnsweredMessages,
            int     iUrgentAnsweredMessages,
            int     iTotalDeletedMessages,
            int     iUrgentDeletedMessages)
    {
        VoicemailNotifyStateListener voicemailNotify = getInstance(false);
        if (voicemailNotify != null)
            voicemailNotify.updateDetailMessageWaiting(strURL, strMessageMediaType, bAbsoluteValues, iTotalNewMessages, iTotalOldMessages, iTotalUntouchedMessages, iUrgentUntouchedMessages, iTotalSkippedMessages, iUrgentSkippedMessages, iTotalFlaggedMessages, iUrgentFlaggedMessages, iTotalReadMessages, iUrgentReadMessages, iTotalAnsweredMessages, iUrgentAnsweredMessages, iTotalDeletedMessages, iUrgentDeletedMessages) ;
        else
            System.out.println("VoicemailNotifyStateListener doDetailMessageWaiting called before object initialized!\n");

    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Called when there is a change in voice mail status.  This is a simple
     * callback that will be used if/when the voice mail provider does not
     * support more detail.
     */
    public void updateBinaryMessageWaiting(String strURL,
            boolean bNewMessages)
    {
        if (m_fiBinary != null) {

            // Uninstall Detailed notification
            if (m_fiDetail != null)
            {
                try
                {
                    m_fiDetail.uninstall() ;
                    Shell.getInstance().setMessageIndicator(PhoneLamp.LAMPMODE_OFF, "DetailVoicemail") ;
                }
                catch (Exception e)
                {
                    SysLog.log(e) ;
                }
            }

            // Update binary notification
            m_fiBinary.setUser(strURL) ;
            m_fiBinary.setState(bNewMessages) ;
        }
    }


    public void updateDetailMessageWaiting(String strURL,
            String  strMessageMediaType,
            boolean bAbsoluteValues,
            int     iTotalNewMessages,
            int     iTotalOldMessages,
            int     iTotalUntouchedMessages,
            int     iUrgentUntouchedMessages,
            int     iTotalSkippedMessages,
            int     iUrgentSkippedMessages,
            int     iTotalFlaggedMessages,
            int     iUrgentFlaggedMessages,
            int     iTotalReadMessages,
            int     iUrgentReadMessages,
            int     iTotalAnsweredMessages,
            int     iUrgentAnsweredMessages,
            int     iTotalDeletedMessages,
            int     iUrgentDeletedMessages)
    {
        if (m_fiDetail != null) {

            // Uninstall Binary notification
            if (m_fiBinary != null)
            {
                try
                {
                    m_fiBinary.uninstall() ;
                    Shell.getInstance().setMessageIndicator(PhoneLamp.LAMPMODE_OFF, "BinaryVoicemail") ;
                }
                catch (Exception e)
                {
                    SysLog.log(e) ;
                }
            }

            // Update Detailed notification
            m_fiDetail.setUser(strURL) ;
            m_fiDetail.setTotals(bAbsoluteValues, iTotalNewMessages, iTotalOldMessages) ;
        }
    }



//////////////////////////////////////////////////////////////////////////////
// Nested/Inner Classes
////
    private class icBinaryMessageFeatureIndicator implements FeatureIndicator
    {
    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
        private boolean m_bInstalled = false ;
        private boolean m_bMessagesWaiting = false ;
        private String  m_strUser ;

        public icBinaryMessageFeatureIndicator()
        {
        }

    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////
        public Image getIcon()
        {
            return null ;
        }

        public String getShortDescription()
        {
            return null ;
        }

        public Component getComponent()
        {
            PComponent compRC = null ;

            if (m_bMessagesWaiting) {
                if (m_strVoicemailRetrieveURL != null)
                    compRC = new PLabel("New Voicemail", PLabel.ALIGN_EAST) ;
                else
                    compRC = new PLabel("New Voicemail") ;

                compRC.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_LARGE)) ;
            } else {
                if (m_strVoicemailRetrieveURL != null)
                    compRC = new PLabel("No Voicemail", PLabel.ALIGN_EAST) ;
                else
                    compRC = new PLabel("No Voicemail") ;

                compRC.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_LARGE)) ;
            }

            return compRC ;
        }


        public String getHint()
        {
            return null ;
        }


        public void buttonPressed()
        {
            if (m_strVoicemailRetrieveURL != null)
            {
                DialingStrategy strategy = Shell.getInstance().getDialingStrategy() ;
                strategy.dial(new PAddress(m_strVoicemailRetrieveURL)) ;
            }
        }


        public void setUser(String strUser)
        {
            m_strUser = strUser ;
        }


        public void setState(boolean bNewMessages)
        {
            m_bMessagesWaiting = bNewMessages ;
            if (m_bMessagesWaiting) {
                install() ;
                Shell.getInstance().setMessageIndicator(m_iMWILampMode, "BinaryVoicemail") ;
            } else {
                if (m_bShouldDisplayNoVoicemail)
                    install() ;
                else
                    uninstall() ;

                Shell.getInstance().setMessageIndicator(PhoneLamp.LAMPMODE_OFF, "BinaryVoicemail") ;
            }
        }


    //////////////////////////////////////////////////////////////////////////
    // Implementation
    ////

        /**
         * Install if not already installed, otherwise refresh
         */
        public void install()
        {
            if (!m_bInstalled) {
                Shell.getFeatureIndicatorManager().installIndicator(this, FeatureIndicatorManager.VIEWSTYLE_LINE) ;
                m_bInstalled = true ;
            } else {
                Shell.getFeatureIndicatorManager().refreshIndicator(this) ;
            }
        }


        /**
         * Uninstall if installed
         */
        public void uninstall()
        {
            if (m_bInstalled) {
                Shell.getFeatureIndicatorManager().removeIndicator(this) ;
                m_bInstalled = false ;
            }
        }
    }


    /**
     *
     *
     */
    private class icDetailMessageFeatureIndicator implements FeatureIndicator
    {
    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
        private boolean m_bInstalled = false ;
        private String  m_strUser ;
        private int     m_iTotalNewMessages = 0 ;
        private int     m_iTotalOldMessages = 0 ;


        public icDetailMessageFeatureIndicator()
        {
        }

    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////
        public Image getIcon()
        {
            return null ;
        }

        public String getShortDescription()
        {
            return null ;
        }


        public Component getComponent()
        {
            String strVoicemail = "No Voicemail" ;

            PComponent compRC = null ;


            if (m_iTotalNewMessages > 0)
            {
                strVoicemail = "New Voicemail: " + m_iTotalNewMessages ;
            }


            if (m_strVoicemailRetrieveURL != null)
                compRC = new PLabel(strVoicemail, PLabel.ALIGN_EAST) ;
            else
                compRC = new PLabel(strVoicemail) ;

            compRC.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_LARGE)) ;

            return compRC ;
        }


        public String getHint()
        {
            return null ;
        }


        public void buttonPressed()
        {
            if (m_strVoicemailRetrieveURL != null) {
                try {
                    PCall call = PCall.createCall() ;
                    call.connect(PAddressFactory.getInstance().createAddress(m_strVoicemailRetrieveURL)) ;
                }
                catch (PSTAPIException e)
                {
                    MessageBox  msgBox = new MessageBox(ShellApp.getInstance().getCoreApp()) ;

                    msgBox.setMessage("Unable to contact voicemail: " + e.toString()) ;
                    msgBox.showModeless() ;
                }
            }
        }


        public void setUser(String strUser)
        {
            m_strUser = strUser ;
        }


        public void setTotals(boolean bAbsoluteValues,
                              int     iTotalNewMessages,
                              int     iTotalOldMessages)
        {
            if (bAbsoluteValues) {
                m_iTotalNewMessages = iTotalNewMessages ;
                m_iTotalOldMessages = iTotalOldMessages ;
            } else {
                m_iTotalNewMessages += iTotalNewMessages ;
                m_iTotalOldMessages += iTotalOldMessages ;
            }

            if (m_iTotalNewMessages < 0)
                m_iTotalNewMessages = 0 ;
            if (m_iTotalOldMessages < 0)
                m_iTotalOldMessages = 0 ;

            if (m_iTotalNewMessages > 0) {
                install() ;
                Shell.getInstance().setMessageIndicator(m_iMWILampMode, "DetailVoicemail") ;
            } else {
                if (m_bShouldDisplayNoVoicemail)
                    install() ;
                else
                    uninstall() ;

                Shell.getInstance().setMessageIndicator(PhoneLamp.LAMPMODE_OFF, "DetailVoicemail") ;
            }
        }


    //////////////////////////////////////////////////////////////////////////
    // Implementation
    ////

        /**
         * Install if not already installed, otherwise refresh
         */
        public void install()
        {
            if (!m_bInstalled) {
                Shell.getFeatureIndicatorManager().installIndicator(this, FeatureIndicatorManager.VIEWSTYLE_LINE) ;
                m_bInstalled = true ;
            } else {
                Shell.getFeatureIndicatorManager().refreshIndicator(this) ;
            }
        }


        /**
         * Uninstall if installed
         */
        public void uninstall()
        {
            if (m_bInstalled) {
                Shell.getFeatureIndicatorManager().removeIndicator(this) ;
                m_bInstalled = false ;
            }
        }
    }
}
