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

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.app.adjustvolume.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;

import org.sipfoundry.stapi.* ;

/**
 * Users can change various volume (and contrast) settings.  This app makes
 * use of a polymorphic AdjustVolumeForm that is reused for all of the volume
 * settings along with contrast adjustment.
 * <p>
 * Apon activing this application, the adjustment screen is selected by
 * context, whereas:
 * <ul>
 *   <li>Ringer Volume, if the ringer is enabled</li>
 *   <li>Base Volume, if the speaker phone is enabled</li>
 *   <li>Handset Volume, if the handset is enabled</li>
 *   <li>ELSE, LCD contrast</li>
 * </ul>
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ChangeVolumeApp extends Application
{
    protected int getInitialContext()
    {
        int iInitialContext = AdjustVolumeForm.Contrast;

        if (PingerApp.isTestbedWorld())
            iInitialContext = AdjustVolumeForm.Other;

        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;

        if (asc != null) {
            switch (asc.getCurrentMedia()) {
                case AudioSourceControl.ASC_HANDSET:
                    iInitialContext = AdjustVolumeForm.HandsetSpeaker ;
                    break ;
                case AudioSourceControl.ASC_SPEAKER_PHONE:
                    iInitialContext = AdjustVolumeForm.BaseSpeaker ;
                    break ;
                case AudioSourceControl.ASC_HEADSET:
                    iInitialContext = AdjustVolumeForm.HeadsetSpeaker ;
                    break ;
                case AudioSourceControl.ASC_RINGER:
                    iInitialContext = AdjustVolumeForm.Ringer ;
                    break ;
            }
        }
        return iInitialContext ;
    }


    public void main(String argv[])
    {
        int iContext;
        if (argv[0] != null && argv[0].length() > 0)
        {
            try
            {
                iContext = Integer.parseInt(argv[0]);
            }
            catch (Exception e)
            {
                if (PingerApp.isTestbedWorld() != true)
                    iContext = AdjustVolumeForm.Contrast;
                else
                    iContext = AdjustVolumeForm.Other;
            }
        }
        else
        {
                if (PingerApp.isTestbedWorld() != true)
                    iContext = AdjustVolumeForm.Contrast;
                else
                    iContext = AdjustVolumeForm.Other;
        }

        if (PingerApp.isTestbedWorld() == true && iContext == AdjustVolumeForm.Contrast)
            iContext = AdjustVolumeForm.BaseSpeaker;


        if (iContext != AdjustVolumeForm.None) {
            if (iContext == AdjustVolumeForm.Other) {
                // Toss up the Select Adjustment Form
                SelectAdjustmentForm form = new SelectAdjustmentForm(this) ;
                switch (form.showModal()) {
                    case SelectAdjustmentForm.SEL_NONE:
                        break ;
                    case SelectAdjustmentForm.SEL_BASE:
                        displayAdjustLevel(AdjustVolumeForm.BaseSpeaker) ;
                        break ;
                    case SelectAdjustmentForm.SEL_CONTRAST:
                        displayAdjustLevel(AdjustVolumeForm.Contrast) ;
                        break ;
                    case SelectAdjustmentForm.SEL_HANDSET:
                        displayAdjustLevel(AdjustVolumeForm.HandsetSpeaker) ;
                        break ;
                    case SelectAdjustmentForm.SEL_RINGER:
                        displayAdjustLevel(AdjustVolumeForm.Ringer) ;
                        break ;
                    case SelectAdjustmentForm.SEL_HEADSET:
                        displayAdjustLevel(AdjustVolumeForm.HeadsetSpeaker) ;
                        break ;
                }
            } else {
                // Toss up the adjust volume form
                displayAdjustLevel(iContext);
            }
        }
    }


    public void displayAdjustLevel(int iContext)
    {
        // Toss up the adjust volume form
        AdjustVolumeForm form = new AdjustVolumeForm(this, true) ;
        form.setContext(iContext) ;
        form.showModal() ;
    }
}
