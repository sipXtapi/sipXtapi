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

import java.util.* ;
import java.io.* ;

import javax.telephony.* ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.phone.* ;
import org.sipfoundry.telephony.phone.event.* ;
import org.sipfoundry.stapi.event.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.service.Logger ;
import org.sipfoundry.util.SysLog;
import org.sipfoundry.sipxphone.sys.app.* ;


/**
 * The PMediaManager is the collection point for all of the audio devices.
 * It offers methods for playing sound files and DTMF tones even if the phone
 * is not currently in a call. It also allows the programmer to change the audio
 * device in use, such as switching between the speakerphone and the handset.
 * <p>
 * A PAudioDevice is a logical sound device that has a generic interface for
 * adjusting volume.  This implementation assumes that only one audio device can
 * be enabled at any one time.
 * <p>
 * This interface will expand over time to address additional audio features
 * available in the phone.
 * <p>
 * To use this class, first get an instance of the class by using the
 * <i>getMediaManager</i> method in {@link Shell}.
 *
 * @see org.sipfoundry.stapi.PAudioDevice
 * @see org.sipfoundry.sipxphone.sys.Shell#getMediaManager
 */
public class PMediaManager
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected PAudioDevice m_devices[] ;
    protected PCall        m_hackCall = null ;
    protected Vector       m_vListenerList ;
    protected AudioSourceControl m_asc ;
    protected int          m_iLastDeviceId ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * protected constructor guards against user construction
     */
    protected PMediaManager()
    {
        m_asc = PhoneHelper.getInstance().getAudioSourceControl() ;

        m_devices = new PAudioDevice[5] ;
        
		m_iLastDeviceId = PAudioDevice.ID_NONE ;

        // Init ID_NONE device
        m_devices[PAudioDevice.ID_NONE] = null ;

        // Init ID_SPEAKER_PHONE device
        m_devices[PAudioDevice.ID_SPEAKER_PHONE] =
                new PAudioDevice(PAudioDevice.ID_SPEAKER_PHONE, "Speaker Phone") ;

        // Init ID_HAND_SET device
        m_devices[PAudioDevice.ID_HAND_SET] =
                new PAudioDevice( PAudioDevice.ID_HAND_SET, "Hand Set") ;

        // Init ID_HEAD_SET device
        m_devices[PAudioDevice.ID_HEAD_SET] =
                new PAudioDevice( PAudioDevice.ID_HEAD_SET, "Head Set") ;

        // Init ID_RINGER device
        m_devices[PAudioDevice.ID_RINGER] =
                new PAudioDevice( PAudioDevice.ID_RINGER, "Ringer") ;



        // Add terminal Listener to pick up new calls!
        Terminal terminal = ShellApp.getInstance().getTerminal() ;
        if (terminal != null) {
            try {
                terminal.addTerminalListener(new icTerminalComponentMonitor()) ;
            } catch (Exception e) {
                System.out.println("STAPI: ERROR: PMediaManager cannot add terminal listener:") ;
                SysLog.log(e) ;
            }
        } else {
            System.out.println("STAPI: ERROR: PMediaManager cannot get terminal reference.") ;
        }
    }



//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Play sound from a file. The audio file must be a raw audio file
     * format with 8000 samples/second (Hz), 16 bit sample size (signed,
     * little endian), one channel only.
     * <p>
     * All sounds and tones played prior to invoking the <i>playSound</i>
     * method are implicitly stopped.
     * <p>
     * The sound will be played on the active audio device or on the
     * ID_SPEAKER_PHONE if no audio device is active. The sound is only played
     * locally. If you would like to play a sound so that a participant
     * in the call can hear the sound, use the <i>playSound</i> method on
     * {@link PCall}.
     *
     * @param strRAWFile The fully qualified file name of the desired RAW sound
     *        file. See method description for sound file specifications.
     * @param bRepeat The repeat flag. If this flag is true, the audio clip will be
     *      played in a loop until stopped. If this flag is false, the audio clip is played once.
     *
     * @exception PMediaLockedException Thrown if the current audio device is
     *            being exclusively used by another application.
     * @exception PMediaNotSupportedException Thrown if the audio file is
     *            invalid or not supported.
     * @exception PMediaUnavailableException Thrown if specified media is
     *            unavailable.
     *
     * @see PCall#playSound
     */
    public void playSound(String strRAWFile, boolean bRepeat)
        throws PMediaLockedException, PMediaNotSupportedException, PMediaUnavailableException
    {
        if (Logger.isEnabled())
        {
            Logger.post("media",
                    Logger.TRAIL_NOTIFICATION,
                    "mm_playSound_file",
                    strRAWFile,
                    String.valueOf(bRepeat)) ;
        }

        PCallManager callManager = STAPIFactory.getInstance().getCallManager() ;

        PCall call = callManager.getInFocusCall() ;
        if (call == null) {
            try {
                m_hackCall = call = PCall.createCall() ;
                callManager.ignoreCall(m_hackCall) ;
                setDefaultAudioDevice(true) ;
            } catch (PCallException e) {
                MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                messageBox.setMessage(messageBox.getString("lblInternalErrorCommandAborted")) ;
                messageBox.showModal() ;
            }
        }

        if (call != null) {
            try {
                call.playSound(strRAWFile, bRepeat, true, false) ;
            } catch (PCallStateException e) {
                SysLog.log(e) ;
            }
        }

        if (m_hackCall != null) {
            try {
                m_hackCall.stopSound() ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }

            try {
                m_hackCall.disconnect() ;
            } catch (PCallException e) {
                SysLog.log(e) ;
            }
            m_hackCall = null ;
        }
    }


    /**
     * Play audio from a stream.
     * <p>
     * All sounds and tones played prior to invoking the <i>playSound</i>
     * method are implicitly stopped.
     * <p>
     * The sound will be played on the active audio device or on the
     * ID_SPEAKER_PHONE if no audio device is active. The sound is only played
     * locally. If you would like to play a sound so that a participant
     * in the call can hear the sound, use the use the <i>playSound</i> method on PCall.
     * <p>
     * To stop playing an audio clip before the clip has reached the end, use the
     * {@link #stopSound stopSound()} method. You may also use the <i>stopSound</i>
     * method to stop playing a looping audio clip.
     * <p>
     * In this release, the audio file must be a raw audio file
     * format with 8000 samples/second (Hz), 16 bit sample size (signed,
     * little endian), one channel only.
     *
     * @param inputStream The stream containing the audio clip. Use
     *      <pre>getApplication().getResourceAsStream(audioResourceName)</pre>
     *      in your form to get the audio clip stream.
     * @param bRepeat The repeat flag. If this flag is true, the audio clip will be
     *      played in a loop until stopped. If this flag is false, the audio clip is played once.
     *
     * @exception PMediaLockedException Thrown if the current audio device is
     *            being exclusively used by another application.
     * @exception PMediaNotSupportException Thrown if the audio file is
     *            invalid or not supported.
     * @exception PMediaUnavailableException Thrown if specified media is
     *            unavailable.
     *
     * @see PCall#playSound
     * @see #stopSound
     * @see org.sipfoundry.sipxphone.Application#getResourceAsStream
     */
    public void playSound(InputStream inputStream, boolean bRepeat)
        throws PMediaLockedException, PMediaNotSupportedException, PMediaUnavailableException
    {
        if (Logger.isEnabled())
        {
            Logger.post("media",
                    Logger.TRAIL_NOTIFICATION,
                    "mm_playSound_stream",
                    inputStream.toString(),
                    String.valueOf(bRepeat)) ;
        }

        PCallManager callManager = STAPIFactory.getInstance().getCallManager() ;

        PCall call = callManager.getInFocusCall() ;
        if (call == null) {
            try {
                m_hackCall = call = PCall.createCall() ;
                setDefaultAudioDevice(true) ;
            } catch (PCallException exception) {
                MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                messageBox.setMessage(messageBox.getString("lblInternalErrorCommandAborted")) ;
                messageBox.showModal() ;
            } catch (PMediaException e) {
                SysLog.log(e) ;
            }

        }

        if (call != null) {
            try {
                call.playSound(inputStream, bRepeat, true, false) ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
        }

        if (m_hackCall != null) {
            try {
                m_hackCall.stopSound() ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
            try {
                m_hackCall.disconnect() ;
            }  catch (PSTAPIException e) {
                SysLog.log(e) ;

            }
            m_hackCall = null ;
        }
    }


    /**
     * Stop playing all audio clips.  If no audio clips are playing,
     * this method does nothing (silent error).
     */
    public void stopSound()
    {
        PCallManager callManager = STAPIFactory.getInstance().getCallManager() ;

        if (Logger.isEnabled())
        {
            Logger.post("media",
                    Logger.TRAIL_NOTIFICATION,
                    "mm_stopSound") ;
        }

        PCall call = callManager.getInFocusCall() ;
        if (call != null) {
            try {
                call.stopSound() ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
        }

        if (m_hackCall != null) {
            try {
                m_hackCall.stopSound() ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }

            try {
                m_hackCall.disconnect() ;
            } catch (PCallException e) {
                SysLog.log(e) ;
            }

            m_hackCall = null ;
        }
    }


    /**
     * Play a DTMF tone until told to stop.  The tone will be played until
     * explicitly stopped by calling <i>stopTone</i>.
     * <p>
     * The tone will be played on the active audio device or on the
     * ID_SPEAKER_PHONE if none are active.  The tone is only played
     * locally. If you would like to play a tone so that a participant
     * in the call can hear the sound, use the <i>playTone</i> method on PCall.
     * <p>
     * To stop playing a tone, use the <i>stopTone</i> method.
     *
     * @param iToneID The tone ID.  See DTMF_* for the values to pass in to this method.
     *
     * @exception IllegalStateException Thrown if the passed iToneID is
     *            invalid.
     * @exception PMediaLockedException Thrown if the current audio device is
     *            being exclusively used by another application.
     *
     * @see PCall#playTone
     * @see #stopTone
     */
    public void playTone(int iToneID)
        throws IllegalStateException, PMediaLockedException
    {
        if (Logger.isEnabled())
        {
            Logger.post("media",
                    Logger.TRAIL_NOTIFICATION,
                    "mm_playTone",
                    String.valueOf(iToneID)) ;
        }

        PCallManager callManager = STAPIFactory.getInstance().getCallManager() ;

        PCall call = callManager.getInFocusCall() ;
        if (call == null) {
            try {
                m_hackCall = call = PCall.createCall() ;
                setDefaultAudioDevice(true) ;
            } catch (PCallException e) {
                MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                messageBox.setMessage(messageBox.getString("lblInternalErrorCommandAborted")) ;
                messageBox.showModal() ;
            }
        }

        if (call != null) {
            try {
                call.playTone(iToneID) ;
            } catch (PCallException e) {
                SysLog.log(e) ;
            }
        }
    }

    /**
     * Play a DTMF tone for a specified duration. The tone will be played for
     * the period specified by iDuration. Tones can be prematurely ended by
     * calling <i>stopTone</i>.
     *
     * The tone will be played on the active audio device or on the
     * ID_SPEAKER_PHONE if none are active. The tone is only played
     * locally. If you would like to play a tone such that a participant
     * in the call can hear the sound, use the <i>playTone</i> method on
     * {@link PCall}.
     *
     * @param iToneID The tone ID. See DTMF_* for the values to pass in to this method.
     * @param iDuration How long the tone should be played in milliseconds.
     *
     * @exception IllegalStateException Thrown if the passed iToneID is
     *            invalid or iDuration is less than 0.
     * @exception PMediaLockedException Thrown if the current audio device is
     *            being exclusively used by another application.
     *
     * @see #stopTone
     * @see PCall#playTone
     */
    public void playTone(int iToneID, int iDuration)
        throws IllegalStateException, PMediaLockedException
    {
        if (Logger.isEnabled())
        {
            Logger.post("media",
                    Logger.TRAIL_NOTIFICATION,
                    "mm_playTone",
                    String.valueOf(iToneID),
                    String.valueOf(iDuration)) ;
        }

        PCallManager callManager = STAPIFactory.getInstance().getCallManager() ;

        PCall call = callManager.getInFocusCall() ;
        if (call == null) {
            try {
                m_hackCall = call = PCall.createCall() ;
                setDefaultAudioDevice(true) ;
            } catch (PCallException e) {
                MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                messageBox.setMessage(messageBox.getString("lblInternalErrorCommandAborted")) ;
                messageBox.showModal() ;
            } catch (PMediaException e) {
                SysLog.log(e) ;
            }
        }

        if (call != null) {
            try {
                call.playTone(iToneID, iDuration) ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
        }

        if (m_hackCall != null) {

            try {
                m_hackCall.stopTone() ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }

            try {
                m_hackCall.disconnect() ;
            } catch (PCallException e) {
                SysLog.log(e) ;
            }
            m_hackCall = null ;
        }
    }


    /**
     * Stop playing a DTMF tone. This method will stop tones started by
     * both the <i>playTone</i> methods. If no tones are playing,
     * this method does nothing (silent error).
     *
     * @see #playTone
     */
    public void stopTone()
    {
        if (Logger.isEnabled())
        {
            Logger.post("media",
                    Logger.TRAIL_NOTIFICATION,
                    "mm_stopTone") ;
        }

        PCallManager callManager = STAPIFactory.getInstance().getCallManager() ;

        PCall call = callManager.getInFocusCall() ;
        if (call != null) {
            try {
                call.stopTone() ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
        }

        if (m_hackCall != null) {
            try {
                m_hackCall.stopSound() ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }

            try {
                m_hackCall.disconnect() ;
            } catch (PCallException e) {
                SysLog.log(e) ;
            }
            m_hackCall = null ;
        }
    }


    /**
     * Set the current audio device.
     *
     * @param iAudioDeviceID The ID of the desired audio device to switch to.
     *
     * @exception IllegalArgumentException Thrown if an invalid audio device
     *            ID is passed in.
     * @exception SecurityException Thrown if the calling user/application
     *            does not have permission to change the audio device.
     * @exception PMediaLockedException Thrown if the specified media device
     *            is being exclusively used by another application.
     */
    public void setAudioDevice(int iAudioDeviceID)
        throws IllegalArgumentException, SecurityException, PMediaLockedException
    {
        PAudioDevice oldAudioDevice ;
        PAudioDevice newAudioDevice ;
        boolean      bMuted = false ;

        if (Logger.isEnabled())
        {
            Logger.post("media",
                    Logger.TRAIL_NOTIFICATION,
                    "mm_setAudioDevice",
                    String.valueOf(iAudioDeviceID)) ;
        }

        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;
        switch (iAudioDeviceID) {
            case PAudioDevice.ID_NONE:
                {
                    // Disabling the auto device.  Make sure we unmute the device
                    // before disabling.
                    oldAudioDevice = getAudioDevice() ;
                    if ((oldAudioDevice != null) && oldAudioDevice.isMuted()) {
                        try {
                            oldAudioDevice.unmute() ;
                        } catch (PMediaStateException e) { /* burp */ }
                    }
                    asc.enableHandset(false) ;
                    asc.enableHeadset(false) ;
                    asc.enableSpeakerPhone(false) ;
                    asc.enableRinger(false) ;

                    PAudioDeviceListener listeners[] = getAudioDeviceListeners() ;
                    for (int i=0; i<listeners.length; i++) {
                        listeners[i].audioDeviceChanged(new PAudioDeviceEvent(null)) ;
                    }
                }
                break ;
            case PAudioDevice.ID_HAND_SET:
                // Enabling the handset; disabling any other control.
                if (asc.getCurrentMedia() != AudioSourceControl.ASC_HANDSET) {
                    // Get the old audio device
					oldAudioDevice = getAudioDevice() ;					

                    // Disable all the auto devices
                    asc.enableHandset(false) ;
                    asc.enableHeadset(false) ;
                    asc.enableSpeakerPhone(false) ;
                    asc.enableRinger(false) ;

                    // Note if the old auto device was muting and unmute it.
                    if ((oldAudioDevice != null) && oldAudioDevice.isMuted()) {
                        bMuted = true ;
                        try {
                            oldAudioDevice.unmute() ;
                        } catch (PMediaStateException e) { /* burp */ }
                    }


                    // Enable the new media device, and maintain the mute setting.
                    asc.enableHandset(true) ;
                    newAudioDevice = getAudioDevice() ;
                    if ((bMuted) && (newAudioDevice != null)) {
                        try {
                            newAudioDevice.mute() ;
                        } catch (PMediaStateException e) { /* burp */ }
                    }

                    // Tell user about the change
                    PAudioDeviceListener listeners[] = getAudioDeviceListeners() ;
                    for (int i=0; i<listeners.length; i++) {
                        listeners[i].audioDeviceChanged(new PAudioDeviceEvent(newAudioDevice)) ;
                    }
                }
                break ;
            case PAudioDevice.ID_HEAD_SET:
                if (asc.getCurrentMedia() != AudioSourceControl.ASC_HEADSET) {
                    // Get the old audio device
					oldAudioDevice = getAudioDevice() ;
					m_iLastDeviceId = iAudioDeviceID ;

                    // Disable all the auto devices
                    asc.enableHandset(false) ;
                    asc.enableHeadset(false) ;
                    asc.enableSpeakerPhone(false) ;
                    asc.enableRinger(false) ;

                    // Note if the old auto device was muting and unmute it.
                    if ((oldAudioDevice != null) && oldAudioDevice.isMuted()) {
                        bMuted = true ;
                        try {
                            oldAudioDevice.unmute() ;
                        } catch (PMediaStateException e) { /* burp */ }
                    }


                    // Enable the new media device, and maintain the mute setting.
                    asc.enableHeadset(true) ;
                    newAudioDevice = getAudioDevice() ;
                    if ((bMuted) && (newAudioDevice != null)) {
                        try {
                            newAudioDevice.mute() ;
                        } catch (PMediaStateException e) { /* burp */ }
                    }

                    // Tell user about the change
                    PAudioDeviceListener listeners[] = getAudioDeviceListeners() ;
                    for (int i=0; i<listeners.length; i++) {
                        listeners[i].audioDeviceChanged(new PAudioDeviceEvent(newAudioDevice)) ;
                    }
                }
                break ;
            case PAudioDevice.ID_SPEAKER_PHONE:
                if (asc.getCurrentMedia() != AudioSourceControl.ASC_SPEAKER_PHONE) 
                {
                    // Get the old audio device
					oldAudioDevice = getAudioDevice() ;
					m_iLastDeviceId = iAudioDeviceID ;

                    // Disable all the auto devices
                    asc.enableHandset(false) ;
                    asc.enableHeadset(false) ;
                    asc.enableSpeakerPhone(false) ;
                    asc.enableRinger(false) ;

                    // Note if the old auto device was muting and unmute it.
                    if ((oldAudioDevice != null) && oldAudioDevice.isMuted()) {
                        bMuted = true ;
                        try {
                            oldAudioDevice.unmute() ;
                        } catch (PMediaStateException e) { /* burp */ }
                    }


                    // Enable the new media device, and maintain the mute setting.
                    asc.enableSpeakerPhone(true) ;
                    newAudioDevice = getAudioDevice() ;
                    if ((bMuted) && (newAudioDevice != null)) {
                        try {
                            newAudioDevice.mute() ;
                        } catch (PMediaStateException e) { /* burp */ }
                    }

                    // Tell user about the change
                    PAudioDeviceListener listeners[] = getAudioDeviceListeners() ;
                    for (int i=0; i<listeners.length; i++) {
                        listeners[i].audioDeviceChanged(new PAudioDeviceEvent(newAudioDevice)) ;
                    }
                }
                break ;
            case PAudioDevice.ID_RINGER:
                if (asc.getCurrentMedia() != AudioSourceControl.ASC_RINGER) {
                    // Get the old audio device
                    oldAudioDevice = getAudioDevice() ;

                    // Disable all the auto devices
                    asc.enableHandset(false) ;
                    asc.enableHeadset(false) ;
                    asc.enableSpeakerPhone(false) ;
                    asc.enableRinger(false) ;

                    // Note if the old auto device was muting and unmute it.
                    if ((oldAudioDevice != null) && oldAudioDevice.isMuted()) {
                        bMuted = true ;
                        try {
                            oldAudioDevice.unmute() ;
                        } catch (PMediaStateException e) { /* burp */ }
                    }


                    // Enable the new media device, and maintain the mute setting.
                    asc.enableRinger(true) ;
                    newAudioDevice = getAudioDevice() ;
                    if ((bMuted) && (newAudioDevice != null)) {
                        try {
                            newAudioDevice.mute() ;
                        } catch (PMediaStateException e) { /* burp */ }
                    }

                    // Tell user about the change
                    PAudioDeviceListener listeners[] = getAudioDeviceListeners() ;
                    for (int i=0; i<listeners.length; i++) {
                        listeners[i].audioDeviceChanged(new PAudioDeviceEvent(newAudioDevice)) ;
                    }

                }
                break ;
            default:
                throw new IllegalArgumentException("illegal audio device id") ;
        }
    }


    /**
     * @deprecated do not expose
     * sets the default audio device.
     * @param bAllowRinger It doesn not allow the ringer to
     * be set as the default audio device if bAllowRinger is false.
     */
    public void setDefaultAudioDevice(boolean bAllowRinger)
        throws IllegalArgumentException, SecurityException, PMediaLockedException
    {
        int iDevice = getAudioDeviceID() ;
	
		/*
		 * When setting the default audio device, the following code looks 
		 * first at the hook switch state.  If the handset is off hook, then
		 * we enable the handset, otherwise, we enable either the speaker
		 * or the headset.
		 */

        if (!PhoneState.getInstance().isOnHook()) 
        {
        	// OFF HOOK
        	        	
			if (iDevice == PAudioDevice.ID_SPEAKER_PHONE)
			{
				// It is possible to be on "speaker" and still have the 
				// handset offhook.				            
				setAudioDevice(PAudioDevice.ID_SPEAKER_PHONE) ;
			}
			else
			{
				// Normal case: enable the handset
				setAudioDevice(PAudioDevice.ID_HAND_SET) ;
			}
        }
        else
        {
        	// ON HOOK
        	
            if (	(iDevice == PAudioDevice.ID_HEAD_SET)
            	||	(iDevice == PAudioDevice.ID_HAND_SET)
				||	(iDevice == PAudioDevice.ID_SPEAKER_PHONE)
				||	((iDevice == PAudioDevice.ID_RINGER) && bAllowRinger))
            {
            	// If headset, speaker, headset, ringer is set, keep it.
                setAudioDevice(iDevice) ;
            }
            else if ((m_iLastDeviceId == PAudioDevice.ID_HEAD_SET)
            	||	(m_iLastDeviceId == PAudioDevice.ID_SPEAKER_PHONE))
            {            
                setAudioDevice(m_iLastDeviceId) ;
            }
            else
            {
				setAudioDevice(PAudioDevice.ID_SPEAKER_PHONE) ;
            }
        } 
    }


    /**
     * @deprecated do not expose
     * sets the default audio device. It doesn not allow the ringer to
     * be set as the default audio device.
     */
    public void setDefaultAudioDevice()
        throws IllegalArgumentException, SecurityException, PMediaLockedException
    {
        setDefaultAudioDevice( false );
    }


    /**
     * Query the active audio device ID.
     *
     * @return The ID of the active audio device.
     *
     * @exception SecurityException thrown if the user/application does not
     *            have permission for this request.
     */
    public int getAudioDeviceID()
        throws SecurityException
    {
        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;
        int iAudioDeviceID = PAudioDevice.ID_NONE ;

        switch (asc.getCurrentMedia()) {
            case AudioSourceControl.ASC_NONE:
                break ;
            case AudioSourceControl.ASC_HEADSET:
                iAudioDeviceID = PAudioDevice.ID_HEAD_SET ;
                break ;
            case AudioSourceControl.ASC_RINGER:
                iAudioDeviceID = PAudioDevice.ID_RINGER ;
                break ;
            case AudioSourceControl.ASC_SPEAKER_PHONE:
                iAudioDeviceID = PAudioDevice.ID_SPEAKER_PHONE ;
                break ;
            case AudioSourceControl.ASC_HANDSET:
                iAudioDeviceID = PAudioDevice.ID_HAND_SET ;
                break ;
        }
        return iAudioDeviceID ;
    }


    /**
     * Query the active audio device.
     *
     * @return The {@link PAudioDevice} object representing the active audio device.
     *
     * @exception SecurityException thrown if the user/application does not
     *            have permission for this request.
     */
    public PAudioDevice getAudioDevice()
        throws SecurityException
    {
        return m_devices[getAudioDeviceID()] ;
    }



    /**
     * Query all audio devices. This method returns an array of the audio devices
     * in the system. Each element in the array represents a single audio device.
     *
     * @return The PAudioDevice array of all audio devices in the system.
     *
     * @exception SecurityException thrown if the user/application does not
     *            have permission for this request.
     */
    public PAudioDevice[] getAudioDevices()
        throws SecurityException
    {
        PAudioDevice rc[] ;

        rc = new PAudioDevice[m_devices.length-1] ;
        for (int i=0;i<m_devices.length-1; i++) {
            rc[i] = m_devices[i+1] ;
        }

        return rc ;
    }


    /**
     * Add an audio device listener to the media manager. After adding, the listener
     * will receive notification when a new audio device is selected and when
     * the volume of the various audio devices changes.
     *
     * @param listener The audio device listener that will receive notifications.
     */
    public void addAudioDeviceListener(PAudioDeviceListener listener)
    {
        if (m_vListenerList == null) {
            m_vListenerList = new Vector() ;
        }

        synchronized (m_vListenerList) {
            if (!m_vListenerList.contains(listener)) {
                m_vListenerList.addElement(listener) ;
            }
        }
    }


    /**
     * Remove the PAudioDeviceListener such that it no longer receives
     * notification when a new audio device is selected, or when
     * the volume of the various audio devices changes.
     *
     * @param listener The audio device listener that wishes to suspend notifications.
     */
    public void removeAudioDeviceListener(PAudioDeviceListener listener)
    {
        if (m_vListenerList != null) {
            synchronized (m_vListenerList) {
                m_vListenerList.removeElement(listener) ;
            }
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Query all of the registered media listeners.
     *
     * @return The array holding registered media listeners.
     */
    protected PAudioDeviceListener[] getAudioDeviceListeners()
    {
        PAudioDeviceListener[] rc = null ;

        // If we have a list return that list other wise return an empty array
        if (m_vListenerList != null) {
            synchronized (m_vListenerList) {

                int iItems = m_vListenerList.size() ;

                rc = new PAudioDeviceListener[iItems] ;
                for (int i=0;i<iItems;i++) {
                    rc[i] = (PAudioDeviceListener) m_vListenerList.elementAt(i) ;
                }
            }
        } else {
            rc = new PAudioDeviceListener[0] ;
        }
        return rc ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////
    protected class icTerminalComponentMonitor extends PtTerminalComponentAdapter
    {
        /**
         *
         */
        public icTerminalComponentMonitor()
        {
            super(IGNORE_BUTTON_EVENTS | IGNORE_HOOKSWITCH_EVENTS) ;
        }


        /**
         *
         */
        public void phoneHandsetVolumeChanged(PtComponentIntChangeEvent event)
        {
            PAudioDevice devices[] = getAudioDevices() ;

            PAudioDeviceListener listeners[] = getAudioDeviceListeners() ;
            for (int i=0; i<listeners.length; i++) {
                listeners[i].volumeChanged(new PAudioDeviceEvent(devices[PAudioDevice.ID_HAND_SET])) ;
            }
        }


        /**
         *
         */
        public void phoneSpeakerVolumeChanged(PtComponentIntChangeEvent event)
        {
            PAudioDevice devices[] = getAudioDevices() ;

            PAudioDeviceListener listeners[] = getAudioDeviceListeners() ;
            for (int i=0; i<listeners.length; i++) {
                listeners[i].volumeChanged(new PAudioDeviceEvent(devices[PAudioDevice.ID_SPEAKER_PHONE])) ;
            }
        }


        /**
         *
         */
        public void phoneMicrophoneGainChanged(PtComponentIntChangeEvent event)
        {
            PAudioDevice devices[] = getAudioDevices() ;

            PAudioDeviceListener listeners[] = getAudioDeviceListeners() ;
            for (int i=0; i<listeners.length; i++) {
                switch (getAudioDeviceID()) {
                    case PAudioDevice.ID_SPEAKER_PHONE:
                        listeners[i].volumeChanged(new PAudioDeviceEvent(devices[PAudioDevice.ID_SPEAKER_PHONE])) ;
                        break ;
                    case PAudioDevice.ID_HAND_SET:
                        listeners[i].volumeChanged(new PAudioDeviceEvent(devices[PAudioDevice.ID_HAND_SET])) ;
                        break ;
                    case PAudioDevice.ID_HEAD_SET:
                        listeners[i].volumeChanged(new PAudioDeviceEvent(devices[PAudioDevice.ID_HEAD_SET])) ;
                        break ;
                }
            }
        }
   }
}
