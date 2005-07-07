/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/CallProviderListenerExt.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
package org.sipfoundry.siplite;

/**
 * The CallProviderListenerExt Interface listens for all new outgoing Calls.
 * Since this interface extends CallProviderListener, it also listens to all
 * new incoming calls.
 *  <br><br>
 * The CallProviderListenerExt Interface is implemented by the application so
 * that all new Calls can be handled as neccessary. <br><br>
 * A CallProviderListenerExt implementation must be registered with the
 * CallProvider in order that events can be recieved by the listening
 * application. <br><br>
 *
 * <blockquote><code> callProvider.addCallProviderListener(callProviderListenerExt)
 * </blockquote></code> <br>
 *
 * Multiple CallProviderListenersExt can be supported by the CallProvider so
 * that CallProviderListeners which perform different tasks can be added or
 * removed as neccessary.
 *
 * @see javax.siplite.CallProviderListener
 * Created: Thu Mar 14 14:20:33 2002
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public interface CallProviderListenerExt
    extends javax.siplite.CallProviderListener{
    /**
     * This method will be invoked by the CallProvider when a new outgoing
	   * Call is created in the stack.
     * It is recommended that you set the DialogListener in the given Dialog
     * to allow for events which occur immediatly.
     * @param    call            The new outgoing Call
     */
    void outgoingCall
        (javax.siplite.Call call );

}// CallProviderListenerExt
