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
 * This interface is used to listen for call state changes in the system.
 * You use this listener to listen for state changes in calls, such as when a
 * call is created, when it is ringing, and when it is hung up.
 *
 * @see PCallManager
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface PConnectionListener
{
    /**
     * A call has been created by the PCall <i>createCall</i> method.  The call 
     * is in the idle state and the {@link PAddress} within the event object is 
     * not yet valid.
     *
     * @param event The {@link PConnectionEvent} object containing both the call 
     *        and address objects associated with the state change, and a 
     *        cause ID if a failure has occurred.
     */
    public void callCreated(PConnectionEvent event) ;
    
        
    /**
     * A call has been destroyed and is no longer valid.  This is an 
     * appropriate time to remove listeners and close applications/forms that
     * are monitoring specific calls.
     *
     * @param event The {@link PConnectionEvent} object containing both the call 
     *        and address objects associated with the state change, and a 
     *        cause ID if a failure has occurred.
     */
    public void callDestroyed(PConnectionEvent event) ;
            

    /**
     * A call/address <i>connect</i> attempt has been made; however, the
     * target has not yet responded. The state can change to outbound 
     * alerting, connected, or failed.
     *
     * @param event The {@link PConnectionEvent} object containing both the call and 
     *        address objects associated with the state change, and a cause ID if a
     *        failure has occurred.
     */
    public void connectionTrying(PConnectionEvent event) ;
    
    /**
     * A call/address <i>connect</i> has been acknowledged by the target and is
     * alerting (or ringing). The state can change to disconnected, 
     * connected, or failed.
     *
     * @param event The {@link PConnectionEvent} object containing both the call and 
     *        address objects associated with the state change, and a cause ID if a
     *        failure has occurred.
     */
    public void connectionOutboundAlerting(PConnectionEvent event) ;
    
    
    /**
     * An inbound call has been detected and it is ringing at the local terminal.  
     * The user can pick up the phone to answer, or the call can be answered under
     * program control by calling <i>answer</i> on the call object held within the event 
     * object.  The state can change to connected, failed, or disconnected.
     *
     * @param event The PConnectionEvent object containing both the call and 
     *        address objects associated with the state change, and a cause ID if a
     *        failure has occurred.
     * @see PConnectionEvent#getCall
     * @see PCall#answer
     */
    public void connectionInboundAlerting(PConnectionEvent event) ;        
 
    /**
     * A connection has been established.  The state can 
     * change to disconnected, held, or failed.
     *
     *
     * @param event The PConnectionEvent object containing both the call and 
     *        address objects associated with the state change, and a cause ID if a
     *        failure has occurred.
     */
    public void connectionConnected(PConnectionEvent event) ;


    /**
     * A connection has failed for one of various reasons.  See the cause 
     * within the connection event for more information.  The state of the call will not
     * change from this call state.
     * @param event The PConnectionEvent object containing both the call and 
     *        address objects associated with the state change, and a cause ID if a
     *        failure has occurred.
     */
    public void connectionFailed(PConnectionEvent event) ;
     
     
    /**
     * A connection has moved into an unsupported or unknown state.  See the 
     * cause code within the connection event for more information.  This
     * state should not be considered a failure and most applications should
     * ignore this state change.
     * 
     * @param event The PConnectionEvent object containing both the call and 
     *        address objects associated with the state change, and a cause ID
     *        if one is available.
     */
    public void connectionUnknown(PConnectionEvent event) ;
     
     
    /**
     * A connection has been disconnected for one of various reasons.  See the
     * cause within the connection event for more information.  The state of the call will not
     * not change from this call state.
     *
     * @param event PConnectionEvent object containing both the call and 
     *        address objects associated with the state change, and a cause ID if a
     *        failure has occurred.
     */
    public void connectionDisconnected(PConnectionEvent event) ;    


    /**
     * The entire call (all connections) has been placed on hold.  At this 
     * point, no audio will be exchanged between the participants of the call.
     * The state can change to connected, failed, or dropped from this call state.
     *
     * @param event PConnectionEvent object containing both the call and 
     *        address objects associated with the state change, and a cause ID if a
     *        failure has occurred.
     */
    public void callHeld(PConnectionEvent event) ;    

    /**
     * A call has been released from (taken off) hold.  In this call state, audio 
     * is once again exchanged between all of the participants of the call.  
     * The state is connected and can change to disconnected, failed, or dropped 
     * from this call state.
     *
     * @param event PConnectionEvent object containing both the call and 
     *        address objects associated with the state change, and a cause ID if a
     *        failure has occurred.
     */
    public void callReleased(PConnectionEvent event) ;    
}
