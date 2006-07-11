/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/hook/NewCallHookData.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.hook;

import org.sipfoundry.stapi.* ;

/**
 * The NewCallHook allows developers to customize the behavior of the xpressa
 * phone when the core software creates a new call for the purpose of dialing.
 * This occurs when the end user lifts the handset, presses the SPEAKER
 * button, presses the HEADSET button, or presses the New Call soft button.
 * <p>
 * The default hook will create a new call, enable an audio device, play the dial
 * tone and launch the dialer application (if no other dialing applications
 * are active).
 * <p>
 * The NewCallHookData object is a simple extension of the hook data object
 * that adds methods to set and retrieve the requested audio device. The
 * default implementation will enable this audio device as part of its
 * processing.
 *
 * @see org.sipfoundry.sipxphone.hook.Hook
 * @see org.sipfoundry.sipxphone.sys.HookManager
 *
 * @since 2.0.0
 */
public class NewCallHookData extends HookData
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    private final String EXCEPTION_INVALID_AUDIO_DEVICE_ID =
            "invalid audio device id" ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private int m_iAudioDeviceID ;  // Audio device ID


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs a new NewCallHookData object.
     *
     * @param iAudioDeviceID ID of the audio device that should be enabled
     *        after creating a new call.
     *
     * @exception IllegalArgumentException Thrown if specified audio device
     *            ID is invalid.
     *
     * @see org.sipfoundry.stapi.PAudioDevice
     */
    public NewCallHookData(int iAudioDeviceID)
        throws IllegalArgumentException
    {
        setAudioDeviceID(iAudioDeviceID) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Get the desired audio device ID for the new call. The default hook will
     * create a call and enable this audio device if no other hooks take
     * control and terminate the chain of responsibility.
     *
     * @return The desired audio device ID for the new call.
     */
    public int getAudioDeviceID()
    {
        return m_iAudioDeviceID ;
    }


    /**
     * Set the desired audio device ID for the new call.
     *
     * @param iAudioDeviceID ID of the audio device that should be enabled
     *        after creating a new call.
     *
     * @exception IllegalArgumentException Thrown if specified audio device
     *            ID is invalid.
     *
     * @see org.sipfoundry.stapi.PAudioDevice
     */
    public void setAudioDeviceID(int iAudioDeviceID)
        throws IllegalArgumentException
    {
        if ((iAudioDeviceID < PAudioDevice.ID_NONE) ||
                (iAudioDeviceID > PAudioDevice.ID_RINGER))
        {
            throw new IllegalArgumentException(EXCEPTION_INVALID_AUDIO_DEVICE_ID) ;
        }

        m_iAudioDeviceID = iAudioDeviceID ;
    }
}
