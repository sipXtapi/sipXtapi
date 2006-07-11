/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/service/WaiterControl.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.service;


/**
 * A simple interface that allows an entity using the "waiter" to control
 * its own destiny.  If implemented, the waiter will only interrupt a
 * process who reports true to "canAbort".  resetCanAbort() is called
 * whenever the Waiter begins sleeping/waiting.
 */
public interface WaiterControl
{
    /**
     * Can/should the waiter abort this task?
     */
    boolean canAbort() ;


    /**
     * Request a reset on your position regarding restarting.  The Waiter will
     * ask the WaiterControl to allow aborts before each time it begins
     * waiting.
     */
    void resetCanAbort(boolean bCanAbort) ;
}
