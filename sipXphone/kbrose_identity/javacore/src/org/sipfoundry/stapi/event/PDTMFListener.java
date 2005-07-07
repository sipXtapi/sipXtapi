/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/stapi/event/PDTMFListener.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
 
package org.sipfoundry.stapi.event;

/**
 * A PDTMFListener receives buttonDown and buttonUp events whenever a remote
 * party generates out-of-band DTMF (Dual Tone Multiple Frequency) according
 * to RFC 2833.
 *
 * Listeners are added to a PCall object and apply to all remote parties.
 *
 * @see PCall
 * @see PDTMFEvent
 *
 * @author Pradeep Paudyal
 */

public interface PDTMFListener
{
    /**
     * This method is invoked when a remote party begins playing a DTMF tone. 
     * This generally occurs when the remote party presses a button on
     * the remote phone.
     *
     * @param event The PDTMFEvent containing details on which button was
     *        pressed and the source of the DTMF (which remote party).
     */
    public void buttonDown( PDTMFEvent event );


    /**
     * This method is invoked when a remote party stops playing a DTMF tone. 
     * This generally occurs when the remote party releases a button that is 
     * being held down on the remote phone.
     *
     * @param event The PDTMFEvent containing details on which button was
     *        depressed and the source of the DTMF (which remote party).
     */
    public void buttonUp( PDTMFEvent event );
}
