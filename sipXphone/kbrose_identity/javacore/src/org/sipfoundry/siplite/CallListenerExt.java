/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/CallListenerExt.java#2 $
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
 * The CallListenerExt Interface listens for all outgoing.
 * calllegs. Since this interface extends CallListener, it also listens
 * to all incoming calllegs.<br><br>
 * The CallListenerExt Interface is implemented by the application so that all
 * new Dialogs within a call can be handled as neccessary. <br><br>
 * A CallListenerExt implementation must be registered with the Call in order that
 * events can be recieved by the listening application. <br><br>
 *
 * <blockquote><code> call.addCallListener(callListenerExt) </blockquote></code>
 * <br>
 *
 * Multiple CallListenersExt can be supported by the Call so that Listeners which
 * perform different tasks can be added or removed as neccessary.
 *
 * @see javax.siplite.CallListener
 * Created: Thu Mar 14 14:20:33 2002
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */


public interface CallListenerExt
    extends javax.siplite.CallListener{

    /**
     * This method will be invoked by the Call when a new outgoing Dialog is
     * received from the stack.
     * @param    dialog	 		The new Dialog that has been created
     */
    void outgoingDialog
        (javax.siplite.Dialog callleg );
}// CallListenerExt
