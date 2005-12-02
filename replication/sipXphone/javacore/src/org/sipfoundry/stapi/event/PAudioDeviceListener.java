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
 * This interface is used to listen for changes in an audio device in use by the
 * application.
 *
 * @see PMediaManager
 * @see PAudioDeviceEvent
 * @see PAudioDevice
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface PAudioDeviceListener
{
    /**
     * This method is called when a new audio device has been selected.  The 
     * {@link PAudioDeviceEvent} includes a reference to the newly selected 
     * {@link PAudioDevice}.
     */
    void audioDeviceChanged(PAudioDeviceEvent event) ;

    /**
     * This method is called when the volume has been changed on an audio device.  Muting is
     * considered a volume change.  
     */
    void volumeChanged(PAudioDeviceEvent event) ;
}
