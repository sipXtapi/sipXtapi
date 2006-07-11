/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/transfer/TransferControllerListener.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.app.transfer ;

import org.sipfoundry.stapi.* ;


/**
 * The transfer controller listener is designed to communicate the state/status
 * of a transfer to any interested parties.
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface TransferControllerListener
{

    /**
     * Invoked if the transfer controller successfully consults with the
     * transfer target.
     */
    public void consultSucceeded(TransferControllerEvent event) ;


    /**
     * Invoked if the transfer controller fails to consult with the transfer
     * target
     */
    public void consultFailed(TransferControllerEvent event) ;



    /**
     * Invoked if the transfer controller hangs up during the consult period
     * before the transfer target answers the call
     */
    public void consultAborted(TransferControllerEvent event) ;



    /**
     * Invoked if the transfer has been completed and all parties have been
     * successfully transferred to the intended target.
     */
    public void transferSucceeded(TransferControllerEvent event) ;


    /**
     * Invoked if the transfer fails for any reason, excluding the transfer
     * controller aborting.
     */
    public void transferFailed(TransferControllerEvent event) ;


    /**
     * Invoked if the transfer is aborted by the transfer controller.
     */
    public void transferAborted(TransferControllerEvent event) ;


    /**
     * Invoked if the transferee hangs up in the middle of a transfer attempt.
     */
    public void transfereeHungup(TransferControllerEvent event) ;


    /**
     * Invoked if the transfer target hangs up during the consult period of the
     * transfer.
     */
    public void transferTargetHungup(TransferControllerEvent event) ;

}
