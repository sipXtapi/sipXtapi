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

package org.sipfoundry.stapi ;

import org.sipfoundry.stapi.event.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.service.Logger ;
import org.sipfoundry.util.SysLog;

import javax.telephony.* ;
import javax.telephony.phone.* ;


/**
 * An audio device is a logical representation of a speaker/microphone pair.
 * Examples of audio devices include the handset, which contains both a
 * microphone and a speaker, and the speakerphone in the base of the phone.
 * <p>
 * You can obtain a reference to a PAudioDevice by using PMediaManager's
 * <i>getAudioDevice</i> method.
 * <p>
 * Upon gaining a reference to a PAudioDevice, you can adjust volume on the
 * speaker and mute/unmute the microphone.
 *
 * @see PMediaManager
 */
public class PAudioDevice
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** No audio device. */
    public static final int ID_NONE = 0 ;
    /** The speakerphone audio device in the base of the phone. */
    public static final int ID_SPEAKER_PHONE = 1 ;
    /** The handset audio device.*/
    public static final int ID_HAND_SET = 2 ;
    /** The headset audio device. */
    public static final int ID_HEAD_SET = 3 ;
    /** The ringer audio device. */
    public static final int ID_RINGER   = 4 ;


    /** The minimum volume setting. */
    public static final int VOLUME_MIN     = 1 ;
    /** The mid-range volume setting. */
    public static final int VOLUME_MID     = 5 ;
    /** The loudest volume setting. */
    public static final int VOLUME_FULL    = 10 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes ;
////
    /** int id of this audio device, must be on of the ID_ constants */
    protected int    m_iID ;
    /** string representation of the device (for user) */
    protected String m_strID ;
    /** gain level at time of muting */
    protected int     m_iGainLevel ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * protected constructor to avoid random construction.  Only the
     * PMediaManager or classes living in the org.sipfoundry.teleping.phone
     * package should construction these objects.
     */
    protected PAudioDevice(int iID, String strName)
    {
        m_iID = iID ;
        m_strID = strName ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Return the name of the audio device.
     *
     * @return A string description of the device, in English.
     */
    public String getName()
    {
        return m_strID ;
    }


    /**
     * Return the ID of the audio device.
     *
     * @return The ID of the audio device.
     * @see #ID_NONE
     * @see #ID_SPEAKER_PHONE
     * @see #ID_HAND_SET
     */
    public int getID()
    {
        return m_iID ;
    }


    /**
     * Query the current volume level setting on the speaker in the audio device.
     *
     * @return The volume setting.  This value will be between 1 and 10,
     * inclusive.
     */
    public int getVolume()
    {
        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;

        switch (m_iID) {
            case ID_SPEAKER_PHONE:
                return asc.getVolume(AudioSourceControl.ASC_SPEAKER_PHONE) ;
            case ID_HAND_SET:
                return asc.getVolume(AudioSourceControl.ASC_HANDSET) ;
            case ID_HEAD_SET:
                return asc.getVolume(AudioSourceControl.ASC_HEADSET) ;
            case ID_RINGER:
                return asc.getVolume(AudioSourceControl.ASC_RINGER) ;
        }

        return 0 ;
    }


    /**
     * Set the desired volume level on the speaker in the audio device.
     * This does not affect the mute setting of the microphone in the audio device.
     *
     * @param iLevel The new volume setting.  The volume should be between
     * 1 and 10 inclusive.
     *
     * @exception IllegalArgumentException Thrown when the volume level
     *            is invalid.
     */
    public void setVolume(int iLevel)
        throws IllegalArgumentException
    {
        if (Logger.isEnabled())
            Logger.post("media", Logger.TRAIL_NOTIFICATION, "setVolume", getName(), String.valueOf(iLevel)) ;

        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;

        // Validate that the volume is within expected bounds.
        if ((iLevel < 1) || (iLevel > 10)) {
            throw new IllegalArgumentException("illegal volume adjustment") ;
        }

        switch (m_iID) {
            case ID_SPEAKER_PHONE:
                {
                    int iOldLevel = asc.getVolume(AudioSourceControl.ASC_SPEAKER_PHONE) ;
                    if (iOldLevel != iLevel) {
                        asc.setVolume(AudioSourceControl.ASC_SPEAKER_PHONE, iLevel) ;
                    }
                }
                break ;
            case ID_HAND_SET:
                {
                    int iOldLevel = asc.getVolume(AudioSourceControl.ASC_HANDSET) ;
                    if (iOldLevel != iLevel) {
                        asc.setVolume(AudioSourceControl.ASC_HANDSET, iLevel) ;
                    }
                }
            case ID_HEAD_SET:
                {
                    int iOldLevel = asc.getVolume(AudioSourceControl.ASC_HEADSET) ;
                    if (iOldLevel != iLevel) {
                        asc.setVolume(AudioSourceControl.ASC_HEADSET, iLevel) ;
                    }
                }
                break ;
            case ID_RINGER:
                {
                    int iOldLevel = asc.getVolume(AudioSourceControl.ASC_RINGER) ;
                    if (iOldLevel != iLevel) {
                        asc.setVolume(AudioSourceControl.ASC_RINGER, iLevel) ;
                    }
                }
                break ;
        }
    }


    /**
     * Mute the microphone in the active audio device, effectively disabling all
     * microphones in the sipXphone phone. This method does not affect the audio or
     * volume setting of the active audio device's speaker.
     *
     * @exception PMediaStateException Thrown if this audio device is already
     *            muted.
     */
    public void mute()
        throws PMediaStateException
    {
        if (Logger.isEnabled())
            Logger.post("media", Logger.TRAIL_NOTIFICATION, "mute", getName()) ;

        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;
        PMediaManager      mediaManager = STAPIFactory.getInstance().getMediaManager() ;
        boolean            bActiveDevice = (m_iID == mediaManager.getAudioDeviceID()) ;

        switch (m_iID) {
            case ID_SPEAKER_PHONE:
                if (asc.getGain(AudioSourceControl.ASC_SPEAKER_PHONE) != 0) {

                    // Set and Update
                    m_iGainLevel = asc.getGain(AudioSourceControl.ASC_SPEAKER_PHONE) ;
                    asc.setGain(AudioSourceControl.ASC_SPEAKER_PHONE, 0) ;
                    PAudioDeviceListener listeners[] = mediaManager.getAudioDeviceListeners() ;
                    for (int i=0; i<listeners.length; i++) {
                        listeners[i].volumeChanged(new PAudioDeviceEvent(mediaManager.getAudioDevice())) ;
                    }
                }
                break ;
            case ID_HAND_SET:
                if (asc.getGain(AudioSourceControl.ASC_HANDSET) != 0) {

                    // Set and Update
                    m_iGainLevel = asc.getGain(AudioSourceControl.ASC_HANDSET) ;
                    asc.setGain(AudioSourceControl.ASC_HANDSET, 0) ;
                    PAudioDeviceListener listeners[] = mediaManager.getAudioDeviceListeners() ;
                    for (int i=0; i<listeners.length; i++) {
                        listeners[i].volumeChanged(new PAudioDeviceEvent(mediaManager.getAudioDevice())) ;
                    }
                }
                break ;
            case ID_HEAD_SET:
                if (asc.getGain(AudioSourceControl.ASC_HEADSET) != 0) {

                    // Set and Update
                    m_iGainLevel = asc.getGain(AudioSourceControl.ASC_HEADSET) ;
                    asc.setGain(AudioSourceControl.ASC_HEADSET, 0) ;
                    PAudioDeviceListener listeners[] = mediaManager.getAudioDeviceListeners() ;
                    for (int i=0; i<listeners.length; i++) {
                        listeners[i].volumeChanged(new PAudioDeviceEvent(mediaManager.getAudioDevice())) ;
                    }
                }
                break ;
            case ID_RINGER:
                if (asc.getGain(AudioSourceControl.ASC_RINGER) != 0) {

                    // Set and Update
                    m_iGainLevel = asc.getGain(AudioSourceControl.ASC_RINGER) ;
                    asc.setGain(AudioSourceControl.ASC_RINGER, 0) ;
                    PAudioDeviceListener listeners[] = mediaManager.getAudioDeviceListeners() ;
                    for (int i=0; i<listeners.length; i++) {
                        listeners[i].volumeChanged(new PAudioDeviceEvent(mediaManager.getAudioDevice())) ;
                    }
                }
                break ;

        }

        // Update MUTE LED
        if (bActiveDevice) {
            PhoneLamp lamp = asc.getButtonLamp("MUTE") ;
            System.out.println("LAMP: " + lamp) ;
            if (lamp != null) {
                try {
                    lamp.setMode(PhoneLamp.LAMPMODE_STEADY) ;
                } catch (InvalidArgumentException e) {
                    SysLog.log(e) ;
                }
            }
        }
    }


    /**
     * Cancel muting of the microphone in the audio device.  This does not
     * t the volume setting of the speaker on the audio device.
     *
     * @exception PMediaStateException Thrown if this audio device is not
     *            muted.
     */
    public void unmute()
        throws PMediaStateException
    {
        if (Logger.isEnabled())
            Logger.post("media", Logger.TRAIL_NOTIFICATION, "unmute", getName()) ;

        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;
        PMediaManager      mediaManager = STAPIFactory.getInstance().getMediaManager() ;
        boolean            bActiveDevice = (m_iID == mediaManager.getAudioDeviceID()) ;

        switch (m_iID) {
            case ID_SPEAKER_PHONE:
                {
                    if (asc.getGain(AudioSourceControl.ASC_SPEAKER_PHONE) == 0) {

                        // Set and update
                        asc.setGain(AudioSourceControl.ASC_SPEAKER_PHONE, m_iGainLevel) ;
                        PAudioDeviceListener listeners[] = mediaManager.getAudioDeviceListeners() ;
                        for (int i=0; i<listeners.length; i++) {
                            listeners[i].volumeChanged(new PAudioDeviceEvent(mediaManager.getAudioDevice())) ;
                        }
                    }
                }
                break ;
            case ID_HAND_SET:
                {
                    if (asc.getGain(AudioSourceControl.ASC_HANDSET) == 0) {

                        // Set and update
                        asc.setGain(AudioSourceControl.ASC_HANDSET, m_iGainLevel) ;
                        PAudioDeviceListener listeners[] = mediaManager.getAudioDeviceListeners() ;
                        for (int i=0; i<listeners.length; i++) {
                            listeners[i].volumeChanged(new PAudioDeviceEvent(mediaManager.getAudioDevice())) ;
                        }
                    }
                }
                break ;
            case ID_HEAD_SET:
                {
                    if (asc.getGain(AudioSourceControl.ASC_HEADSET) == 0) {

                        // Set and update
                        asc.setGain(AudioSourceControl.ASC_HEADSET, m_iGainLevel) ;
                        PAudioDeviceListener listeners[] = mediaManager.getAudioDeviceListeners() ;
                        for (int i=0; i<listeners.length; i++) {
                            listeners[i].volumeChanged(new PAudioDeviceEvent(mediaManager.getAudioDevice())) ;
                        }
                    }
                }
                break ;
            case ID_RINGER:
                if (asc.getGain(AudioSourceControl.ASC_RINGER) == 0) {

                    // Set and Update
                    asc.setGain(AudioSourceControl.ASC_RINGER, m_iGainLevel) ;
                    PAudioDeviceListener listeners[] = mediaManager.getAudioDeviceListeners() ;
                    for (int i=0; i<listeners.length; i++) {
                        listeners[i].volumeChanged(new PAudioDeviceEvent(mediaManager.getAudioDevice())) ;
                    }
                }
                break ;

        }

        // Update Mute LED
        if (bActiveDevice) {
            PhoneLamp lamp = asc.getButtonLamp("MUTE") ;
            System.out.println("LAMP: " + lamp) ;
            if (lamp != null) {
                try {
                    lamp.setMode(PhoneLamp.LAMPMODE_OFF) ;
                } catch (InvalidArgumentException e) {
                    SysLog.log(e) ;
                }
            }
        }
    }


    /**
     * Is the microphone currently muted?
     *
     * @return True if the microphone of the audio device is muted,
     * false otherwise.
     */
    public boolean isMuted()
    {
        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;
        boolean            bMuted = false ;

        switch (m_iID) {
            case ID_SPEAKER_PHONE:
                if (asc.getGain(AudioSourceControl.ASC_SPEAKER_PHONE) == 0)
                    bMuted = true ;
                break ;
            case ID_HAND_SET:
                if (asc.getGain(AudioSourceControl.ASC_HANDSET) == 0)
                    bMuted = true ;
                break ;
            case ID_HEAD_SET:
                if (asc.getGain(AudioSourceControl.ASC_HEADSET) == 0)
                    bMuted = true ;
                break ;
            case ID_RINGER:
                if (asc.getGain(AudioSourceControl.ASC_RINGER) == 0)
                    bMuted = true ;
                break ;


        }
        return bMuted ;
    }
}
