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

import javax.telephony.* ;
import javax.telephony.phone.* ;
import java.util.* ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.phone.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.util.* ;


/**
 * AudioSourceControl is a helper class that abstracts and manages the audio devices
 * on the phone.  This includes activating media devices along with respective volumes.
 * <br><br>
 * Current, we are enforcing a rule that only one audio component can be enabled at a
 * time.  Enabling a device will deactivate any other device that was active.
 */
public class AudioSourceControl
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** audio source: none / idle nothing is enabled */
    public static final int ASC_NONE             = 0 ;
    /** audio source: handset */
    public static final int ASC_HANDSET          = 1 ;
    /** audio source: Speaker Phone */
    public static final int ASC_SPEAKER_PHONE    = 2 ;
    /** audio source: Head Set */
    public static final int ASC_HEADSET          = 3 ;
    /** audio source: ringer */
    public static final int ASC_RINGER           = 4 ;

    /** min supported volume */
    protected static final int MIN_VOLUME     = PhoneSpeaker.MUTE ;
    /** max supported volume */
    protected static final int MAX_VOLUME     = PhoneSpeaker.FULL ;

    /** min supported gain */
    public static final int MIN_GAIN       = PhoneSpeaker.MUTE ;
    /** max supported gain*/
    public static final int MAX_GAIN       = PhoneSpeaker.FULL ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** reference to our terminal */
    protected PhoneTerminal m_terminal ;

    /** cache of LEDs */
    protected Hashtable     m_htLEDCache ;

    /** cache component groups  */
    protected Hashtable     m_htComponentGroupCache ;


    /** what media device is enabled? */
    protected int           iMediaEnabled ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * AudioSourceControl requires a phone terminal to operate
     */
    public AudioSourceControl(PhoneTerminal terminal)
    {
        m_terminal = terminal ;
        m_htLEDCache = new Hashtable() ;
        m_htComponentGroupCache = new Hashtable() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * enable/disable the speaker phone.  If enabling, this will deactivate
     * whatever device was previously enabled.
     */
    public void enableSpeakerPhone(boolean bEnable)
    {
        if (bEnable) {
            // Enable iff not already active
            if (iMediaEnabled != ASC_SPEAKER_PHONE) {
                enableMediaDevice(iMediaEnabled, false) ;
                enableMediaDevice(ASC_SPEAKER_PHONE, true) ;
                iMediaEnabled = ASC_SPEAKER_PHONE ;
            }
        } else {
            // Disable iff active
            if (iMediaEnabled == ASC_SPEAKER_PHONE) {
                enableMediaDevice(ASC_SPEAKER_PHONE, false) ;
                iMediaEnabled = ASC_NONE ;
            }
        }
    }


    public int getCurrentMedia()
    {
        return iMediaEnabled ;
    }


    /**
     * enable/disable the hand set.  If enabling, this will deactivate
     * whatever device was previously enabled.
     */
    public void enableHandset(boolean bEnable)
    {
        if (bEnable) {
            // Enable iff not already active
            if (iMediaEnabled != ASC_HANDSET) {
                enableMediaDevice(iMediaEnabled, false) ;
                enableMediaDevice(ASC_HANDSET, true) ;
                iMediaEnabled = ASC_HANDSET ;
            }
        } else {
            // Disable iff active
            if (iMediaEnabled == ASC_HANDSET) {
                enableMediaDevice(ASC_HANDSET, false) ;
                iMediaEnabled = ASC_NONE ;
            }
        }
    }


    /**
     * enable/disable the head set.  If enabling, this will deactivate
     * whatever device was previously enabled.
     */
    public void enableHeadset(boolean bEnable)
    {
        if (bEnable) {
            // Enable iff not already active
            if (iMediaEnabled != ASC_HEADSET) {
                enableMediaDevice(iMediaEnabled, false) ;
                enableMediaDevice(ASC_HEADSET, true) ;
                iMediaEnabled = ASC_HEADSET ;
            }
        } else {
            // Disable iff active
            if (iMediaEnabled == ASC_HEADSET) {
                enableMediaDevice(ASC_HEADSET, false) ;
                iMediaEnabled = ASC_NONE ;
            }
        }
    }


    public void enableRinger(boolean bEnable)
    {
        if (bEnable) {
            // Enable iff not already active
            if (iMediaEnabled != ASC_RINGER) {
                enableMediaDevice(iMediaEnabled, false) ;
                enableMediaDevice(ASC_RINGER, true) ;
                iMediaEnabled = ASC_RINGER ;
            }
        } else {
            // Disable iff active
            if (iMediaEnabled == ASC_RINGER) {
                enableMediaDevice(ASC_RINGER, false) ;
                iMediaEnabled = ASC_NONE ;
            }
        }
    }


    /**
     * Disable whatever media device is currently active.
     */
    public void disable()
    {
        enableMediaDevice(iMediaEnabled, false) ;
        iMediaEnabled = ASC_NONE ;
    }


    /**
     * enable/disable a specific media device.  This code provides the
     * translation between out logical devices and JTAPI component groups.
     */
    public void enableMediaDevice(int iDevice, boolean bEnable)
    {
        switch (iDevice) {
            case ASC_NONE:
                if (iMediaEnabled == ASC_HANDSET)
                    enableComponentGroup(ComponentGroup.HAND_SET, false) ;
                if (iMediaEnabled == ASC_HEADSET)
                    enableComponentGroup(ComponentGroup.HEAD_SET, false) ;
                if (iMediaEnabled == ASC_SPEAKER_PHONE)
                    enableComponentGroup(ComponentGroup.SPEAKER_PHONE, false) ;
                if (iMediaEnabled == ASC_RINGER)
                    enableComponentGroup(ComponentGroup.PHONE_SET, false) ;
                break ;
            case ASC_HANDSET:
                enableComponentGroup(ComponentGroup.HAND_SET, bEnable) ;
                break ;
            case ASC_SPEAKER_PHONE:
                enableComponentGroup(ComponentGroup.SPEAKER_PHONE, bEnable) ;
                break ;
            case ASC_HEADSET:
                enableComponentGroup(ComponentGroup.HEAD_SET, bEnable) ;
                break ;
            case ASC_RINGER:
                enableComponentGroup(ComponentGroup.PHONE_SET, bEnable) ;
                break ;
        }
    }


    /**
     * get the volume of the default audio device
     */
    public int getVolume()
    {
        return getMediaDeviceVolume(iMediaEnabled) ;
    }

    /**
     * get the default level of the default audio device
     */
    public int getDefaultVolume()
    {
        return getMediaDeviceDefaultVolume(iMediaEnabled) ;
    }



    /**
     * get the gain of the default audio device
     */
    public int getGain()
    {
        return getMediaDeviceGain(iMediaEnabled) ;

    }


    /**
     * get the volume of a particular audio device
     */
    public int getVolume(int iMedia)
    {
        return getMediaDeviceVolume(iMedia) ;
    }


    /**
     * get the default volume of a particular audio device
     */
    public int getDefaultVolume(int iMedia)
    {
        return getMediaDeviceDefaultVolume(iMedia) ;
    }



    public int getGain(int iMedia)
    {
        return getMediaDeviceGain(iMedia) ;
    }


    /**
     * set the volume of the default audio device
     */
    public void setVolume(int iVolume)
    {
        if (iMediaEnabled != ASC_NONE) {
            int iOldVolume = getMediaDeviceVolume(iMediaEnabled) ;
            int iNewVolume = iVolume ;

            // Clip to max volume
            if (iNewVolume > MAX_VOLUME)
                iNewVolume = MAX_VOLUME ;

            // Clip to min volume
            if (iNewVolume < MIN_VOLUME)
                iNewVolume = MIN_VOLUME ;

            if (iNewVolume != iOldVolume) {
                setMediaDeviceVolume(iMediaEnabled, iNewVolume) ;
            }
        }
    }
        /**
     * set the volume of the default audio device
     */
    public void setVolume(int iMedia, int iVolume)
    {
        if (iMedia != ASC_NONE) {
            int iOldVolume = getMediaDeviceVolume(iMedia) ;
            int iNewVolume = iVolume ;

            // Clip to max volume
            if (iNewVolume > MAX_VOLUME)
                iNewVolume = MAX_VOLUME ;

            // Clip to min volume
            if (iNewVolume < MIN_VOLUME)
                iNewVolume = MIN_VOLUME ;

            if (iNewVolume != iOldVolume) {
                setMediaDeviceVolume(iMedia, iNewVolume) ;
            }
        }
    }



    /**
     * set the gain of a particular audio device
     */
    public void setGain(int iMedia, int iGain)
    {
        if (iMedia != ASC_NONE) {
            int iOldGain = getMediaDeviceGain(iMedia) ;
            int iNewGain = iGain ;

            // Clip to max volume
            if (iNewGain > MAX_GAIN)
                iNewGain = MAX_GAIN ;

            // Clip to min volume
            if (iNewGain < MIN_GAIN)
                iNewGain = MIN_GAIN ;

            if (iNewGain != iOldGain) {
                setMediaDeviceGain(iMedia, iNewGain) ;
            }
        }
    }


    /**
     * increase the volume of a particular audio device
     */
    public void incVolume(int iMedia)
    {
        if (iMedia != ASC_NONE) {
            int iOldVolume = getMediaDeviceVolume(iMedia) ;
            int iNewVolume = iOldVolume + 1 ;

            // Clip to min volume
            if (iNewVolume > MAX_VOLUME)
                iNewVolume = MAX_VOLUME ;

            if (iNewVolume != iOldVolume) {
                setMediaDeviceVolume(iMedia, iNewVolume) ;
            }
        }
    }


    /**
     * increase the volume of the currently active audio source.
     */
    public void incVolume()
    {
        incVolume(iMediaEnabled) ;
    }


    /**
     * Decrease the volume of the a paricular audio type
     */
    public void decVolume(int iMedia)
    {
        if (iMedia != ASC_NONE) {
            int iOldVolume = getMediaDeviceVolume(iMedia) ;
            int iNewVolume = iOldVolume - 1 ;

            // Clip to min volume
            if (iNewVolume < MIN_VOLUME)
                iNewVolume = MIN_VOLUME ;

            if (iNewVolume != iOldVolume) {
                setMediaDeviceVolume(iMedia, iNewVolume) ;
            }
        }
    }


    /**
     * Decrease the volume of the a paricular audio type
     */
    public void decVolume()
    {
        decVolume(iMediaEnabled) ;
    }


    /**
     * store/save a snapshot of the current audio levels
     */
    public void saveAudioLevels()
    {
        PropertyManager pm = PropertyManager.getInstance() ;
        AudioLevels     audioLevels = new AudioLevels() ;

        audioLevels.setHandsetVolume(getMediaDeviceVolume(ASC_HANDSET)) ;
        audioLevels.setHeadsetVolume(getMediaDeviceVolume(ASC_HEADSET)) ;
        audioLevels.setRingerVolume(getMediaDeviceVolume(ASC_RINGER)) ;
        audioLevels.setSpeakerVolume(getMediaDeviceVolume(ASC_SPEAKER_PHONE)) ;
        audioLevels.setLCDContrast(LCDContrast.getLCDContrast()) ;

        try {
            pm.ownedBeanChanged(audioLevels) ;
        } catch (Exception e) {
            System.out.println("Error Saving Audio Levels: ") ;
            SysLog.log(e) ;
        }
/*
        System.out.println("AudioSourceControl::Saving Audio Levels...") ;
        System.out.println("\tHeadset:" + audioLevels.getHeadsetVolume()) ;
        System.out.println("\tHandset:" + audioLevels.getHandsetVolume()) ;
        System.out.println("\tRinger:" + audioLevels.getRingerVolume()) ;
        System.out.println("\tSpeaker:" + audioLevels.getSpeakerVolume()) ;
        System.out.println("\tContrast:" + audioLevels.getLCDContrast()) ;
*/
    }


    /**
     * Initialize audio levels to pre-saved ranges
     */
    public void initializeAudioLevels()
    {
        PropertyManager pm = PropertyManager.getInstance() ;
        boolean         bLoaded = false ;

        AudioLevels audioLevels = new AudioLevels() ;

        if (pm.exists(audioLevels)) {
            // Try grabbing the default audio levels from our owned bean
            try {
                audioLevels = (AudioLevels) pm.getOwnedBean(audioLevels) ;

                setMediaDeviceVolume(ASC_HANDSET, audioLevels.getHandsetVolume()) ;
                setMediaDeviceVolume(ASC_HEADSET, audioLevels.getHeadsetVolume()) ;
                setMediaDeviceVolume(ASC_RINGER, audioLevels.getRingerVolume()) ;
                setMediaDeviceVolume(ASC_SPEAKER_PHONE, audioLevels.getSpeakerVolume()) ;
                LCDContrast.setLCDContrast(audioLevels.getLCDContrast()) ;

                bLoaded = true ;

            } catch (Exception e) {
                // we really don't care about the exception...  we know the
                // load failed...  hmm, perhaps this is a bigger problem???
            }
        }

        if (!bLoaded) {
            // It is perfectly valid that the bean does not exist
            audioLevels.setHeadsetVolume(getMediaDeviceVolume(ASC_HEADSET)) ;
            audioLevels.setRingerVolume(getMediaDeviceVolume(ASC_RINGER)) ;
            audioLevels.setSpeakerVolume(getMediaDeviceVolume(ASC_SPEAKER_PHONE)) ;
            audioLevels.setHandsetVolume(getMediaDeviceVolume(ASC_HANDSET)) ;
            audioLevels.setLCDContrast(LCDContrast.getLCDContrast()) ;

            try {
                pm.ownedBeanChanged(audioLevels);
            } catch (Exception e) {
                System.out.println("Error Saving Audio Levels: ") ;
                SysLog.log(e) ;
            }
        }
/*
        System.out.println("AudioSourceControl::Restored Audio Levels") ;
        System.out.println("\tHeadset:" + audioLevels.getHeadsetVolume()) ;
        System.out.println("\tHandset:" + audioLevels.getHandsetVolume()) ;
        System.out.println("\tRinger:" + audioLevels.getRingerVolume()) ;
        System.out.println("\tSpeaker:" + audioLevels.getSpeakerVolume()) ;
        System.out.println("\tContrast:" + audioLevels.getLCDContrast()) ;
*/
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * get the volume for a specific media device.  This code provides the
     * translation between our logical devices and JTAPI component groups.
     */
    protected int getMediaDeviceVolume(int iDevice)
    {
        int iVolume = 0 ;
        switch (iDevice) {
            case ASC_NONE:
                break ;
            case ASC_HANDSET:
                iVolume = getComponentGroupVolume(ComponentGroup.HAND_SET) ;
                break ;
            case ASC_SPEAKER_PHONE:
                iVolume = getComponentGroupVolume(ComponentGroup.SPEAKER_PHONE) ;
                break ;
            case ASC_HEADSET:
                iVolume = getComponentGroupVolume(ComponentGroup.HEAD_SET) ;
                break ;
            case ASC_RINGER:
                iVolume = getComponentGroupVolume(ComponentGroup.PHONE_SET) ;
                break ;

        }
        return iVolume ;
    }

    /**
     * get the default volume for a specific media device.  This code provides
     * the translation between our logical devices and JTAPI component groups.
     */
    protected int getMediaDeviceDefaultVolume(int iDevice)
    {
        int iVolume = 0 ;
        switch (iDevice) {
            case ASC_NONE:
                break ;
            case ASC_HANDSET:
                iVolume = getComponentGroupDefaultVolume(ComponentGroup.HAND_SET) ;
                break ;
            case ASC_SPEAKER_PHONE:
                iVolume = getComponentGroupDefaultVolume(ComponentGroup.SPEAKER_PHONE) ;
                break ;
            case ASC_HEADSET:
                iVolume = getComponentGroupDefaultVolume(ComponentGroup.HEAD_SET) ;
                break ;
            case ASC_RINGER:
                iVolume = getComponentGroupDefaultVolume(ComponentGroup.PHONE_SET) ;
                break ;

        }
        return iVolume ;
    }



    /**
     * get the gain for a specific media device.  This code provides the
     * translation between out logical devices and JTAPI component groups.
     */
    protected int getMediaDeviceGain(int iDevice)
    {
        int iVolume = 0 ;
        switch (iDevice) {
            case ASC_NONE:
                break ;
            case ASC_HANDSET:
                iVolume = getComponentGroupGain(ComponentGroup.HAND_SET) ;
                break ;
            case ASC_SPEAKER_PHONE:
                iVolume = getComponentGroupGain(ComponentGroup.SPEAKER_PHONE) ;
                break ;
            case ASC_HEADSET:
                iVolume = getComponentGroupGain(ComponentGroup.HEAD_SET) ;
                break ;
            case ASC_RINGER:
                iVolume = getComponentGroupGain(ComponentGroup.PHONE_SET) ;
                break ;
        }
        return iVolume ;
    }


    /**
     * set the volume for a specific media device.  This code provides the
     * translation between out logical devices and JTAPI component groups.
     */
    protected void setMediaDeviceVolume(int iDevice, int iVolume)
    {
        switch (iDevice) {
            case ASC_NONE:
                break ;
            case ASC_HANDSET:
                setComponentGroupVolume(ComponentGroup.HAND_SET, iVolume) ;
                break ;
            case ASC_SPEAKER_PHONE:
                setComponentGroupVolume(ComponentGroup.SPEAKER_PHONE, iVolume) ;
                break ;
            case ASC_HEADSET:
                setComponentGroupVolume(ComponentGroup.HEAD_SET, iVolume) ;
                break ;
            case ASC_RINGER:
                setComponentGroupVolume(ComponentGroup.PHONE_SET, iVolume) ;
                break ;
        }
    }

    /**
     * set the gain for a specific media device.  This code provides the
     * translation between out logical devices and JTAPI component groups.
     */
    protected void setMediaDeviceGain(int iDevice, int iGain)
    {
        switch (iDevice) {
            case ASC_NONE:
                break ;
            case ASC_HANDSET:
                setComponentGroupGain(ComponentGroup.HAND_SET, iGain) ;
                break ;
            case ASC_SPEAKER_PHONE:
                setComponentGroupGain(ComponentGroup.SPEAKER_PHONE, iGain) ;
                break ;
            case ASC_HEADSET:
                setComponentGroupGain(ComponentGroup.HEAD_SET, iGain) ;
                break ;
            case ASC_RINGER:
                setComponentGroupGain(ComponentGroup.PHONE_SET, iGain) ;
                break ;
        }
    }


    /**
     * enable/disable the JTAPI component group
     */
    protected void enableComponentGroup(int type, boolean bEnable)
    {
        ComponentGroup group = getComponentGroup(type) ;
        if (group != null) {
            String strDescription = group.getDescription() ;
            if (bEnable) {
                if (Logger.isEnabled())
                {
                    Logger.post("asc", Logger.TRAIL_NOTIFICATION, "enableComponent", strDescription) ;
                }

                /*
                 * Enable the component group
                 */

                if (group.activate()) {
                    System.out.println("ASC: " + strDescription + ": " + "Activated") ;
                } else {
                    System.out.println("ASC: " + strDescription + ": " + "Failed to Activate") ;
                }


                if (type == ComponentGroup.SPEAKER_PHONE)
                {
                    PhoneLamp lampSpeaker = getButtonLamp("SPEAKER") ;
                    if (lampSpeaker != null)
                    {
                        try
                        {
                            lampSpeaker.setMode(PhoneLamp.LAMPMODE_STEADY) ;
                        }
                        catch (InvalidArgumentException e)
                        {
                            SysLog.log(e) ;
                        }
                    }
                }
                else if ((type == ComponentGroup.HEAD_SET) ||
                        ((type == ComponentGroup.HAND_SET) &&
                            PhoneState.getInstance().isHeadsetAsHandset()))
                {
                    PhoneLamp lampHeadset = getButtonLamp("HEADSET") ;
                    if (lampHeadset != null)
                    {
                        try
                        {
                            lampHeadset.setMode(PhoneLamp.LAMPMODE_STEADY) ;
                        }
                        catch (InvalidArgumentException e)
                        {
                            SysLog.log(e) ;
                        }
                    }
                }
            }
            else
            {
                /*
                 * Disable the component group
                 */

                if (Logger.isEnabled())
                {
                    Logger.post("asc", Logger.TRAIL_NOTIFICATION, "disableComponent", strDescription) ;
                }

                if (group.deactivate()) {
                    System.out.println("ASC: " + strDescription + ": " + "Deactivated") ;
                } else {
                    System.out.println("ASC: " + strDescription + ": " + "Failed to deactivate") ;
                }

                if (type == ComponentGroup.SPEAKER_PHONE)
                {
                    PhoneLamp lampSpeaker = getButtonLamp("SPEAKER") ;
                    if (lampSpeaker != null)
                    {
                        try
                        {
                            lampSpeaker.setMode(PhoneLamp.LAMPMODE_OFF) ;
                        }
                        catch (InvalidArgumentException e)
                        {
                            SysLog.log(e) ;
                        }
                    }
                }
                else if (   (type == ComponentGroup.HEAD_SET )  ||
                        ((type == ComponentGroup.HAND_SET) &&
                        PhoneState.getInstance().isHeadsetAsHandset())) ;
                {
                    PhoneLamp lampHeadset = getButtonLamp("HEADSET") ;
                    if (lampHeadset != null)
                    {
                        try
                        {
                            lampHeadset.setMode(PhoneLamp.LAMPMODE_OFF) ;
                        }
                        catch (InvalidArgumentException e)
                        {
                            SysLog.log(e) ;
                        }
                    }
                }
            }
        }
    }


    /**
     * get the volume for a specific component group
     */
    protected int getComponentGroupVolume(int iType)
    {
        int iVolume = 0 ;

        ComponentGroup group = getComponentGroup(iType) ;
        if (group != null) {
            Component components[] = group.getComponents() ;
            for (int j=0; j<components.length; j++) {
                if (components[j] instanceof PhoneSpeaker) {
                    iVolume = ((PhoneSpeaker) components[j]).getVolume() ;
                    break ;
                }
            }
        }

        return iVolume ;
    }

    /**
     * get the default volume for a specific component group
     */
    protected int getComponentGroupDefaultVolume(int iType)
    {
        int iVolume = 0 ;

        ComponentGroup group = getComponentGroup(iType) ;
        if (group != null) {
            Component components[] = group.getComponents() ;
            for (int j=0; j<components.length; j++) {
                if (components[j] instanceof PhoneSpeaker) {
                    iVolume = ((PtPhoneSpeaker) components[j]).getDefaultVolume() ;
                    break ;
                }
            }
        }

        return iVolume ;
    }



    /**
     * get the gain for a specific component group
     */
    protected int getComponentGroupGain(int iType)
    {
        int iGain = 0 ;

        ComponentGroup group = getComponentGroup(iType) ;
        if (group != null) {
            Component components[] = group.getComponents() ;
            for (int j=0; j<components.length; j++) {
                if (components[j] instanceof PhoneMicrophone) {
                    iGain = ((PhoneMicrophone) components[j]).getGain() ;
                    break ;
                }
            }
        }

        return iGain ;
    }



    /**
     * set the volume for a specific component group
     */
    protected void setComponentGroupVolume(int iType, int iVolume)
    {
        boolean bDone = false ;

        ComponentGroup group = getComponentGroup(iType) ;
        if (group != null) {
            String strDescription = group.getDescription() ;
            Component components[] = group.getComponents() ;
            for (int j=0; j<components.length; j++) {
                if (components[j] instanceof PhoneSpeaker) {
                    if (Logger.isEnabled())
                    {
                        Logger.post("asc", Logger.TRAIL_NOTIFICATION, "setVolume", strDescription, String.valueOf(iVolume)) ;
                    }
                    ((PhoneSpeaker) components[j]).setVolume(iVolume) ;
                    bDone = true ;
                    break ;
                }
            }
        }
    }


    /**
     * set the gain for a specific component group
     */
    protected void setComponentGroupGain(int iType, int iGain)
    {
        boolean bDone = false ;

        ComponentGroup group = getComponentGroup(iType) ;
        if (group != null) {
            String strDescription = group.getDescription() ;
            Component components[] = group.getComponents() ;
            for (int j=0; j<components.length; j++) {
                if (components[j] instanceof PhoneMicrophone) {
                    try {
                        if (Logger.isEnabled())
                        {
                            Logger.post("asc", Logger.TRAIL_NOTIFICATION, "setGain", strDescription, String.valueOf(iGain)) ;
                        }
                        ((PhoneMicrophone) components[j]).setGain(iGain) ;
                    } catch (InvalidArgumentException iae) {
                        SysLog.log(iae) ;
                    }
                    bDone = true ;
                    break ;
                }
            }
        }
    }


    public PhoneButton getButton(String strButton)
    {
        PhoneButton button = null ;

        Component components[] = ((PtTerminal) m_terminal).getComponents() ;
        for (int j=0; j<components.length; j++) {
            if ((components[j] != null) && (((PtComponent) components[j]).getType() == 0)) {
                if (((PtPhoneButton) components[j]).getInfo().equalsIgnoreCase(strButton)) {
                    button = (PtPhoneButton) components[j] ;
                    break ;
                }
            }
        }
        return button ;
    }


    /**
     * Get the component group (use cache if possible)
     */
    protected ComponentGroup getComponentGroup(int iType)
    {
        ComponentGroup group = null ;
        group = (ComponentGroup) m_htComponentGroupCache.get(new Integer(iType)) ;
        if (group == null) {
            ComponentGroup groups[] = m_terminal.getComponentGroups() ;
            for (int i=0; (i<groups.length); i++) {
                if (groups[i].getType() == iType) {
                    group = groups[i] ;
                    m_htComponentGroupCache.put(new Integer(iType), group) ;
                    break ;
                }
            }
        }

        return group ;
    }

    public PhoneLamp getButtonLamp(String strButton)
    {
        PhoneLamp lamp = null ;

        lamp = (PhoneLamp) m_htLEDCache.get(strButton) ;
        if (lamp == null) {
            Component components[] = ((PtTerminal) m_terminal).getComponents() ;
            for (int j=0; j<components.length; j++) {
                if ((components[j] != null) && (((PtComponent) components[j]).getType() == 0)) {
                    if (((PtPhoneButton) components[j]).getInfo().equalsIgnoreCase(strButton)) {
                        PtPhoneButton button = (PtPhoneButton) components[j] ;
                        if (button != null) {
                            lamp = button.getAssociatedPhoneLamp() ;
                            if (lamp != null) {
                                m_htLEDCache.put(strButton, lamp) ;
                            }
                        }
                        break ;
                    }
                }
            }
        }
        return lamp ;
    }



    public void dumpComponents()
    {
        ComponentGroup groups[] = m_terminal.getComponentGroups() ;
        for (int i=0; i<groups.length; i++) {
            System.out.println("Group (" + groups[i].getType()+ "): " + groups[i].getDescription()) ;
            Component components[] = groups[i].getComponents() ;
            for (int j=0; j<components.length; j++) {
                System.out.println("index="+j+" comp="+components[j]) ;
                if (components[j] != null) {
                    System.out.println("  Component: type=" + ((PtComponent) components[j]).getType() + " name=" + components[j].getName()) ;
                }
            }
        }
    }

    public void dumpAllComponents()
    {
        Component components[] = ((PtTerminal) m_terminal).getComponents() ;
        for (int j=0; j<components.length; j++) {
            System.out.println("index="+j+" comp="+components[j]) ;
            if (components[j] != null) {
                System.out.println("  Component: type=" + ((PtComponent) components[j]).getType() + " name=" + components[j].getName()) ;
                if (((PtComponent) components[j]).getType() == 0) {
                    PtPhoneButton button = (PtPhoneButton) components[j] ;
                    System.out.println("Button info: " + button.getInfo()) ;
                }
            } else {
                System.out.println("  NULL") ;
            }
        }
    }
}
