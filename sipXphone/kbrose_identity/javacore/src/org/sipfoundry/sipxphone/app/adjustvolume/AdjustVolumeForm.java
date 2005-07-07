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

package org.sipfoundry.sipxphone.app.adjustvolume ;

import java.awt.* ;
import java.awt.event.* ;
import java.util.* ;
import java.io.* ;

import java.text.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.app.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.util.*;
import org.sipfoundry.sipxphone.sys.startup.* ;
import org.sipfoundry.sipxphone.sys.app.core.*;

import org.sipfoundry.stapi.* ;


/**
 * Form that allows the user to alert the volume for the current media device.
 * You are also allowed to change the volume of other events along with LCD
 * contrast.
 * <br><br>
 * This form is reused for all forms of volume settings along with LCD
 * contrast.  Currently, this looks as the current audio state to determine
 * what to adjust.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class AdjustVolumeForm extends SimpleSliderForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final int Ringer          = 0 ;
    public static final int HandsetSpeaker  = 1 ;
    public static final int HeadsetSpeaker  = 2 ;
    public static final int BaseSpeaker     = 3 ;
    public static final int Contrast        = 4 ;
    public static final int Other           = 6 ;
    public static final int None            = 7 ;

    public static final String STRING_RESOURCES = "AdjustVolumeForm.properties" ;

    public static final int MIN_VOLUME      = 0 ;
    public static final int MAX_VOLUME      = 10 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** what is the current context/mode that we are adjusting*/
    protected int m_iCurrentContext ;

    /** what was the init setting of our device? */
    protected int m_iInitialSetting ;

    /** set when the user has cancelled... */
    private boolean m_bCancel ;

    /** Should we allow switching to another audio device? */
    private boolean m_bAllowSwitchingDevices ;

    /** dispatches command (menu & button) to hanlders */
    private icCommandDispatcher m_commandDispatcher ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs and an adjust application form given an application context
     *
     * @param application Application context used to create this form
     * @param bAllowSwitchingDevices Should the user beable to switch to
     *        another audio/lcd device from this form.
     */
    public AdjustVolumeForm(Application application, boolean bAllowSwitchingDevices)
    {
        super(application, "Adjust Level") ;

        m_bCancel = false ;
        m_bAllowSwitchingDevices = bAllowSwitchingDevices ;
        setStringResourcesFile(STRING_RESOURCES) ;

        m_commandDispatcher = new icCommandDispatcher() ;

        initializeCommandbar() ;
        initMenus() ;
    }


    /**
     * Constructs an adjust application form give a parent form
     *
     * @param formParent Parent form of this form.
     * @param bAllowSwitchingDevices Should the user beable to switch to
     *        another audio/lcd device from this form.
     */
    public AdjustVolumeForm(PForm formParent, boolean bAllowSwitchingDevices)
    {
        super(formParent, "Adjust Level") ;

        m_bCancel = false ;
        m_bAllowSwitchingDevices = bAllowSwitchingDevices ;
        setStringResourcesFile(STRING_RESOURCES) ;

        m_commandDispatcher = new icCommandDispatcher() ;

        initializeCommandbar() ;
        initMenus() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * called when the form has lost focus.
     *
     * @param formGainingFocus form that is gaining focus over this form
     */
    public void onFocusLost(PForm formGainingFocus)
    {
        if ((formGainingFocus != null) && (!m_bCancel)) {

            if (    (!(formGainingFocus instanceof MessageBox)) &&
                    (!formGainingFocus.getFormName().equals("Task Manager")) &&
                    (!formGainingFocus.getFormName().equals("Select Adjustment")) &&
                    (!formGainingFocus.getFormName().equals(getTitle()))) {
                onExit() ;
            }
        }

    }


    /**
     *
     */
    public void onUp()
    {
        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;

        switch (m_iCurrentContext) {
            case Contrast:
                LCDContrast.incLCDContrast() ;
                super.onUp() ;
                break ;
            case Ringer:
                asc.incVolume(AudioSourceControl.ASC_RINGER) ;
                super.onUp() ;
                break ;
            case HandsetSpeaker:
                asc.incVolume(AudioSourceControl.ASC_HANDSET) ;
                super.onUp() ;
                break ;
            case HeadsetSpeaker:
                asc.incVolume(AudioSourceControl.ASC_HEADSET) ;
                super.onUp() ;
               break ;
            case BaseSpeaker:
                asc.incVolume(AudioSourceControl.ASC_SPEAKER_PHONE) ;
                super.onUp() ;
                break ;

        }
    }


    /**
     *
     */
    public void onDown()
    {
        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;

        switch (m_iCurrentContext) {
            case Contrast:
                LCDContrast.decLCDContrast() ;
                super.onDown() ;
                break ;
            case Ringer:
                asc.decVolume(AudioSourceControl.ASC_RINGER) ;
                super.onDown() ;
                break ;
            case HandsetSpeaker:
                asc.decVolume(AudioSourceControl.ASC_HANDSET) ;
                super.onDown() ;
                break ;
            case HeadsetSpeaker:
                asc.decVolume(AudioSourceControl.ASC_HEADSET) ;
                super.onDown() ;
                break ;
            case BaseSpeaker:
                asc.decVolume(AudioSourceControl.ASC_SPEAKER_PHONE) ;
                super.onDown() ;
                break ;
        }
    }


    /**
     *
     */
    public void onOther()
    {

        SelectAdjustmentForm formSelectAdjustment = new SelectAdjustmentForm(this) ;

        switch (formSelectAdjustment.showModal()) {
            case SelectAdjustmentForm.SEL_NONE:
                System.out.println("AdjustVolumeForm::onOther(), SEL_NONE") ;
                onCancel() ;
                break ;
            case SelectAdjustmentForm.SEL_BASE:
                setContext(BaseSpeaker) ;
                break ;
            case SelectAdjustmentForm.SEL_CONTRAST:
                setContext(Contrast) ;
                break ;
            case SelectAdjustmentForm.SEL_HANDSET:
                setContext(HandsetSpeaker) ;
                break ;
            case SelectAdjustmentForm.SEL_RINGER:
                setContext(Ringer) ;
                break ;
            case SelectAdjustmentForm.SEL_HEADSET:
                setContext(HeadsetSpeaker) ;
                break ;
        }

        initMenus();
    }


    /**
     *
     */
    public void onTest()
    {
        AudioSourceControl asc =
            PhoneHelper.getInstance().getAudioSourceControl() ;

        String strPath = PingerInfo.getInstance().getDefaultRingFileName();

        try {
            int iDevice = asc.getCurrentMedia() ;
            int iTestDevice = AudioSourceControl.ASC_NONE ;

            asc.enableMediaDevice(iDevice, false) ;
            switch (m_iCurrentContext) {
                case Contrast:
                    break ;
                case Ringer:
                    iTestDevice = AudioSourceControl.ASC_RINGER ;
                    break ;
                case HandsetSpeaker:
                    iTestDevice = AudioSourceControl.ASC_HANDSET ;
                    break ;
                case HeadsetSpeaker:
                    iTestDevice = AudioSourceControl.ASC_HEADSET ;
                    break ;
                case BaseSpeaker:
                    iTestDevice = AudioSourceControl.ASC_SPEAKER_PHONE ;
                    break ;
            }

            asc.enableMediaDevice(iTestDevice, true) ;
            PMediaManager mediaManager = Shell.getMediaManager() ;
            Shell.getInstance().displayStatus("Testing...") ;
            mediaManager.playSound(strPath, false) ;
            Shell.getInstance().clearStatus(0) ;
            asc.enableMediaDevice(iTestDevice, false) ;
            asc.enableMediaDevice(iDevice, true) ;
        } catch (PMediaException e) {
            SysLog.log(e) ;
        }
    }



    /**
     *
     */
    public void onDefault()
    {
        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;

        switch (m_iCurrentContext) {
            case Contrast:
                LCDContrast.setLCDContrast(LCDContrast.getLCDContrastNominal()) ;
                setPosition(LCDContrast.getLCDContrastNominal()) ;
                break ;
            case Ringer:
                asc.setVolume(AudioSourceControl.ASC_RINGER, asc.getDefaultVolume(AudioSourceControl.ASC_RINGER)) ;
                setPosition(asc.getVolume(AudioSourceControl.ASC_RINGER)) ;
                break ;
            case HandsetSpeaker:
                asc.setVolume(AudioSourceControl.ASC_HANDSET, asc.getDefaultVolume(AudioSourceControl.ASC_HANDSET)) ;
                setPosition(asc.getVolume(AudioSourceControl.ASC_HANDSET)) ;
                break ;
            case HeadsetSpeaker:
                asc.setVolume(AudioSourceControl.ASC_HEADSET, asc.getDefaultVolume(AudioSourceControl.ASC_HEADSET)) ;
                setPosition(asc.getVolume(AudioSourceControl.ASC_HEADSET)) ;
               break ;
            case BaseSpeaker:
                asc.setVolume(AudioSourceControl.ASC_SPEAKER_PHONE, asc.getDefaultVolume(AudioSourceControl.ASC_SPEAKER_PHONE)) ;
                setPosition(asc.getVolume(AudioSourceControl.ASC_SPEAKER_PHONE)) ;
                break ;

        }

    }


    /**
     *
     */
    public void onCancel()
    {
        // restore volume
        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;
        m_bCancel = true ;

//System.out.println("AdjustVolumeControl::onCancel(), m_iCurrentContext: " + m_iCurrentContext) ;

        switch (m_iCurrentContext) {
            case Contrast:
                LCDContrast.setLCDContrast(m_iInitialSetting) ;
                break ;
            case Ringer:
                asc.setVolume(AudioSourceControl.ASC_RINGER, m_iInitialSetting) ;
                break ;
            case HandsetSpeaker:
                asc.setVolume(AudioSourceControl.ASC_HANDSET, m_iInitialSetting) ;
                break ;
            case HeadsetSpeaker:
                asc.setVolume(AudioSourceControl.ASC_HEADSET, m_iInitialSetting) ;
                break ;
            case BaseSpeaker:
                asc.setVolume(AudioSourceControl.ASC_SPEAKER_PHONE, m_iInitialSetting) ;
                break ;
            default:
//System.out.println("AdjustVolumeControl::onCancel(), default option") ;
                m_iCurrentContext = None ;
                break;
        }
        closeForm() ;
    }


    /**
     *
     */
    public void onExit()
    {
        PhoneHelper.getInstance().getAudioSourceControl().saveAudioLevels() ;
        m_bCancel = true ;
        closeForm() ;
    }

    /**
     *
     */
    public void onOk()
    {
        PhoneHelper.getInstance().getAudioSourceControl().saveAudioLevels() ;
        super.onOk();
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * initialize the form for the specified context
     */
    public void setContext(int iContext)
    {
        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;

//System.out.println("AdjustVolumeForm::setContext = " + iContext) ;
        m_iCurrentContext = iContext ;

        switch (iContext) {
            case Ringer:
                // Adjust the ringer volume
                setTitle(getString("lblRingerTitle")) ;
                setHelpText(getString("adjust_volume"), getString("adjust_volume_title")) ;
                m_iInitialSetting = asc.getVolume(AudioSourceControl.ASC_RINGER) ;
                setSliderValues(getString("lblMinVol"),
                                getString("lblMaxVol"),
                                MIN_VOLUME,
                                MAX_VOLUME,
                                asc.getVolume(AudioSourceControl.ASC_RINGER)) ;
//System.out.println("AdjustVolumeForm::setContext ") ;
                setInstructions("") ;
                break ;
            case HandsetSpeaker:
                // Adjust the handset volume
                setTitle(getString("lblHandsetSpeakerTitle")) ;
                setHelpText(getString("adjust_volume"), getString("adjust_volume_title")) ;
                m_iInitialSetting = asc.getVolume(AudioSourceControl.ASC_HANDSET) ;
                setSliderValues(getString("lblMinVol"),
                                getString("lblMaxVol"),
                                MIN_VOLUME,
                                MAX_VOLUME,
                                asc.getVolume(AudioSourceControl.ASC_HANDSET)) ;
//System.out.println("AdjustVolumeForm::setContext ") ;
                setInstructions("") ;
                break ;
            case HeadsetSpeaker:
                // Adjust the headset volume
                setTitle(getString("lblHeadsetSpeakerTitle")) ;
                setHelpText(getString("adjust_volume"), getString("adjust_volume_title")) ;
                m_iInitialSetting = asc.getVolume(AudioSourceControl.ASC_HEADSET) ;
                setSliderValues(getString("lblMinVol"),
                                getString("lblMaxVol"),
                                MIN_VOLUME,
                                MAX_VOLUME,
                                asc.getVolume(AudioSourceControl.ASC_HEADSET)) ;
//System.out.println("AdjustVolumeForm::setContext ") ;
                setInstructions("") ;
                break ;
            case BaseSpeaker:
                // Adjust the speaker phone volume
                setTitle(getString("lblBaseSpeakerTitle")) ;
                setHelpText(getString("adjust_volume"), getString("adjust_volume_title")) ;
                m_iInitialSetting = asc.getVolume(AudioSourceControl.ASC_SPEAKER_PHONE) ;
                setSliderValues(getString("lblMinVol"),
                                getString("lblMaxVol"),
                                MIN_VOLUME,
                                MAX_VOLUME,
                                asc.getVolume(AudioSourceControl.ASC_SPEAKER_PHONE)) ;
//System.out.println("AdjustVolumeForm::setContext - base") ;
                setInstructions("") ;
                break ;
            case Contrast:
                // Adjust the LCD Contrast level
                setTitle(getString("lblLCDContrastTitle")) ;
                setHelpText(getString("adjust_lcd_contrast"), getString("adjust_lcd_contrast_title")) ;
                m_iInitialSetting = LCDContrast.getLCDContrast() ;
                setSliderValues(getString("lblMinContrast"),
                                getString("lblMaxContrast"),
                                LCDContrast.getLCDContrastLow(),
                                LCDContrast.getLCDContrastHigh(),
                                LCDContrast.getLCDContrast()) ;
//System.out.println("AdjustVolumeForm::setContext: contrast") ;
                setInstructions(getString("lblContrastHint")) ;
                break ;
            case Other:
                // Select something to change
                onOther() ;
                setInstructions("") ;
                break ;

        }

        m_slider.setLocation(28, 28);

        // make sure that the test button doesn't show for contrast
        if (m_iCurrentContext == Contrast)
        {
            m_bbActions.enableByAction(m_commandDispatcher.ACTION_TEST, false) ;
        }
        else
            m_bbActions.enableByAction(m_commandDispatcher.ACTION_TEST, true) ;

        initMenus();
    }


    private void initializeCommandbar()
    {
        m_bbActions.addButton(  new PLabel(getImage("imgOther")),
                                m_commandDispatcher.ACTION_OTHER,
                                getString("hint/adjustments/change/select")) ;

        if (!m_bAllowSwitchingDevices)
            m_bbActions.enableByAction(m_commandDispatcher.ACTION_OTHER, false);


        if (m_iCurrentContext != Contrast)
        {
            m_bbActions.addButton(  new PLabel(getImage("imgTest")),
                                m_commandDispatcher.ACTION_TEST,
                                getString("hint/adjustments/change/test")) ;
        }

        m_bbActions.addActionListener(m_commandDispatcher) ;
    }

    private void initMenus()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;
        icCommandDispatcher actionListener = new icCommandDispatcher() ;
        getLeftMenuComponent().removeAllItems();
        getRightMenuComponent().removeAllItems();

        PActionItem actionIncrease = new PActionItem( new PLabel("Increase"),
                getString("hint/adjustments/change/level_up"),
                actionListener,
                actionListener.ACTION_INCREASE);
        getLeftMenuComponent().addItem(actionIncrease) ;

        PActionItem actionDecrease = new PActionItem( new PLabel("Decrease"),
                getString("hint/adjustments/change/level_down"),
                actionListener,
                actionListener.ACTION_DECREASE);
        getLeftMenuComponent().addItem(actionDecrease) ;

        PActionItem actionOther = new PActionItem( new PLabel("Other"),
                getString("hint/adjustments/change/select"),
                actionListener,
                actionListener.ACTION_OTHER);

//        if (m_bAllowSwitchingDevices)
        getLeftMenuComponent().addItem(actionOther) ;

        if (m_iCurrentContext != Contrast)
        {
            PActionItem actionTest = new PActionItem( new PLabel("Test"),
                    getString("hint/adjustments/change/test"),
                    actionListener,
                    actionListener.ACTION_TEST);
            getLeftMenuComponent().addItem(actionTest) ;
        }

        PActionItem actionAbout = new PActionItem( new PLabel("About"),
                                    getString("hint/core/system/aboutform"),
                                    m_commandDispatcher,
                                    m_commandDispatcher.ACTION_ABOUT) ;
        getRightMenuComponent().addItem(actionAbout) ;

        PActionItem actionOk = new PActionItem( new PLabel(getString("lblGenericOk")),
                getString("hint/xdk/simplelistform/ok"),
                actionListener,
                actionListener.ACTION_OK) ;
        getRightMenuComponent().addItem(actionOk) ;

        PActionItem actionCancel = new PActionItem( new PLabel(getString("lblGenericCancel")),
                getString("hint/xdk/simplelistform/cancel"),
                actionListener,
                actionListener.ACTION_CANCEL) ;
        getRightMenuComponent().addItem(actionCancel) ;

        PActionItem actionDefault = new PActionItem( new PLabel("Default"),
                getString("hint/adjustments/change/default"),
                actionListener,
                actionListener.ACTION_DEFAULT);
        menuControl.setItem(PBottomButtonBar.B1, actionDefault) ;
        getRightMenuComponent().addItem(actionDefault) ;



    }


//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////
    /**
     *
     */
    protected class icCommandDispatcher implements PActionListener
    {
        public final String ACTION_OK       = "action_ok" ;
        public final String ACTION_CANCEL   = "action_cancel" ;
        public final String ACTION_ABOUT    = "action_about" ;
        public final String ACTION_OTHER    = "action_other" ;
        public final String ACTION_TEST     = "action_test" ;
        public final String ACTION_DEFAULT  = "action_defaults" ;
        public final String ACTION_INCREASE  = "action_increase" ;
        public final String ACTION_DECREASE  = "action_decrease" ;


        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_OK))
            {
                onOk();
                event.consume() ;
            }
            else
            if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel();
                event.consume() ;
            }
            else
            if (event.getActionCommand().equals(ACTION_ABOUT))
            {
                SystemAboutBox box = new SystemAboutBox(getApplication());
                box.onAbout();
                event.consume() ;
            }
            else
            if (event.getActionCommand().equals(ACTION_INCREASE)) {
                onUp() ;
                event.consume() ;
            }
            else if (event.getActionCommand().equals(ACTION_DECREASE)) {
                onDown() ;
                event.consume() ;
            }
            if (event.getActionCommand().equals(ACTION_TEST)) {
                onTest() ;
                event.consume() ;
            }
            else if (event.getActionCommand().equals(ACTION_DEFAULT)) {
                onDefault() ;
                event.consume() ;
            }
            else if (event.getActionCommand().equals(ACTION_OTHER)) {
                onOther() ;
                event.consume() ;
            }

        }
    }


    public class icButtonListener implements PButtonListener
    {
        /**
         * The specified button has been press downwards
         */
        public void buttonDown(PButtonEvent event)
        {
        }


        /**
         * The specified button has been released
         */
        public void buttonUp(PButtonEvent event)
        {
            switch (event.getButtonID()) {
                case PButtonEvent.BID_VOLUME_UP:
                    onUp() ;
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_VOLUME_DN:
                    onDown() ;
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
    }
}
