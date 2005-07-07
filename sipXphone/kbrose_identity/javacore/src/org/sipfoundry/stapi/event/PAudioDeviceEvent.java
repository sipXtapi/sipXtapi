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
 
package org.sipfoundry.stapi.event ;

import org.sipfoundry.stapi.* ;

/**
 * This event is sent when an audio device changes.
 *
 * @see PAudioDevice
 * @see PAudioDeviceListener
 * @see PMediaManager
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PAudioDeviceEvent
{
    protected PAudioDevice m_audioDevice = null ;
    
    
    /**
     * Default Constructor.
     */     
    public PAudioDeviceEvent(PAudioDevice device)
    {
        m_audioDevice = device ; 
        
    }
 
    
    /**
     * The PAudioDevice object that just had a state change.
     */
    public PAudioDevice getAudioDevice()
    {
        return m_audioDevice ;        
    }
}
