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

package org.sipfoundry.sipxphone.hook ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.telephony.PtTerminalConnection;
import java.io.File;
import org.sipfoundry.util.SysLog;

/**
 * DefaultRinger hook used by the xpressa(TM) phone.  The default Ringer hook
 * plays the user's default ring file to the active media device.  If no audio
 * device is active, then the speakerphone ringer is used.
 *
 */
public class DefaultRingerHook implements Hook
{

    /**
     * Required; notify the end user
     * of an incoming call.  The notification can be visual or auditory.
     * Hook authors should call <i>terminate</i> if reacting to the hook.
     */
    public void hookAction(HookData data)
    {
        if ((data != null) && (data instanceof RingerHookData)) {
            RingerHookData hookData = (RingerHookData) data ;

            // Simply play the default ring sound file.
            PMediaManager mediaManager = Shell.getMediaManager() ;
            try {
                //play the ringer file if its exists
                //otherwise play the call waiting tone
                if( (new File(hookData.getDefaultRingfile())).exists() ){
                    mediaManager.playSound
                        (hookData.getDefaultRingfile(), true) ;
                }else{
                    mediaManager.playTone
                        (PtTerminalConnection.DTMF_TONE_RINGTONE) ;
                }
            } catch (PMediaException e) {
                SysLog.log(e) ;
            }
            hookData.terminate() ;
        }
    }
}
