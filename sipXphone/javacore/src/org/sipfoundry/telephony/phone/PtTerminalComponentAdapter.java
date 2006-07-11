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

package org.sipfoundry.telephony.phone ;

import javax.telephony.* ; 
import javax.telephony.events.* ; 
import javax.telephony.phone.events.* ;

import org.sipfoundry.telephony.phone.event.* ;

import org.sipfoundry.sipxphone.service.* ;;


/** 
 * terminal component adapter
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PtTerminalComponentAdapter implements PtTerminalComponentListener
{
    public static final long IGNORE_NONE                 = 0x00000000 ;
    public static final long IGNORE_BUTTON_EVENTS        = 0x00000001 ;
    public static final long IGNORE_HOOKSWITCH_EVENTS    = 0x00000002 ;   
    public static final long IGNORE_VOLUME_EVENTS        = 0x00000004 ;   
    
    long m_lEventFilter = 0  ;
        
    public PtTerminalComponentAdapter()
    {
        m_lEventFilter = IGNORE_NONE ;
    }
    
    
    public PtTerminalComponentAdapter(long lEventFilter)
    {
        m_lEventFilter = lEventFilter ;
    }
    
    
    public long getEventFilter()
    {
        return m_lEventFilter ;
    }
           
    public void terminalListenerEnded(TerminalEvent event) {}
    public void phoneRingerVolumeChanged(PtComponentIntChangeEvent event) {}
    public void phoneRingerPatternChanged(PtComponentIntChangeEvent event) {}
    public void phoneRingerInfoChanged(PtComponentStringChangeEvent event) {}
    public void phoneSpeakerVolumeChanged(PtComponentIntChangeEvent event) {}
    public void phoneMicrophoneGainChanged(PtComponentIntChangeEvent event) {}
    public void phoneLampModeChanged(PtComponentIntChangeEvent event) {}
    public void phoneButtonInfoChanged(PtComponentStringChangeEvent event) {}
    public void phoneButtonUp(PtTerminalComponentEvent event) {}
    public void phoneButtonDown(PtTerminalComponentEvent event) {}
    public void phoneButtonRepeat(PtTerminalComponentEvent event) {}
    public void phoneHookswitchOffhook(PtTerminalComponentEvent event) {}
    public void phoneHookswitchOnhook(PtTerminalComponentEvent event) {}
    public void phoneDisplayChanged(PtTerminalComponentEvent event) {}
    public void phoneHandsetVolumeChanged(PtComponentIntChangeEvent event) {}
}
