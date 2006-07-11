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

import org.sipfoundry.util.SysLog;
import org.sipfoundry.sipxphone.sys.Settings ;
import org.sipfoundry.util.PingerConfig ;


/**
 * Default Match Dialplan hook implementation used by the xpressa(tm) phone.  This
 * implementation simply looks at the length of the dial string.
 */
public class DefaultMatchDialplanHook implements Hook
{
    /**
     * Hook interface method invoked as part of the hook chain.
     */
    public void hookAction(HookData data)
    {
        PingerConfig config = PingerConfig.getInstance() ;
        String       strLength ;
        int          iLength = Settings.getInt("DIALER_PLAN_DIGITS", 4) ;

        // Figure out our default dial plan length
        try {
            strLength = (String) config.getValue("PHONESET_DIALPLAN_LENGTH") ;
            if (strLength != null)
                iLength = Integer.parseInt(strLength) ;
        } catch (Exception e) {
            SysLog.log(e) ;
        }

        // Evaluate dial plan length
        if ((data != null) && (data instanceof MatchDialplanHookData)) {
            MatchDialplanHookData hookData = (MatchDialplanHookData) data ;
            String strDialString = hookData.getDialString() ;

            if (strDialString.length() >= iLength) {
                hookData.setMatchState(MatchDialplanHookData.MATCH_TIMEOUT) ;
            }
        }
    }
}




