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


package org.sipfoundry.sipxphone.sys ;

import org.sipfoundry.stapi.* ;


/**
 * Core applications use a dial strategy when attemping to 'make a call'.
 * This allows the system to change the behavior depending on the current
 * context (i.e. transfer or conference).
 * <br>
 * The Dialing Strategy is hung of the Shell object and applications must
 * make a special effort to use it.
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface DialingStrategy
{
    /**
     * Dial this remote address with the default local address
     * or invoke the strategic method/action
     */
    public void dial(PAddress remoteAddress) ;

    /**
     * Dial this remote address with the specified local address
     * or invoke the strategic method/action
     */
    public void dial(PAddress localAddress, PAddress remoteAddress) ;



    /**
     * this remote address with the default local address or
     * invoke the strategic method/action on a specific call
     */
    public void dial(PAddress remoteAddress, PCall call) ;

    /**
     * Dial this remote address with the specified local address
     * or invoke the strategic method/action on a specific call
     */
    public void dial(PAddress localAddress, PAddress remoteAddress, PCall call);

    /**
     * Aborting the dialing process
     */
    public void abort(PCall call) ;


    /**
     * Aborting the dialing process
     */
    public void abort() ;


    /**
     * Get the string based representation of this strategy.  This text is
     * typically prepended the the form name.
     */
    public String getFunction() ;


    /**
     * Get the string based instructions for this strategy.  This text is
     * typically displayed in lower half of the "dialer" form.
     */
    public String getInstructions() ;


    /**
     * Get the string based representation of the dialing strategy action.
     * This text is typically displayed as the B3 button label
     */
    public String getAction() ;


    /**
     * Get the hint text associated with the dialing strategy action.  This
     * hint text is typically displayed when the B3 buttons is pressed and
     * held down.
     */
    public String getActionHint() ;


    /**
     * Get the cancel status for this dialing strategy.
     *
     * @return boolean true if this operation/strategy can be cancelled,
     *         otherwise false
     */
    public boolean isCancelable() ;


    /**
     * Evaluates if this stategy supports a changeable user.  For example, a
     * new call can be placed as any valid user, however, a specific call leg
     * (as in conference), cannot.  Similary, once could not transfer a call
     * as another user.
     *
     * @return boolean true if this dialing stragegy supports changeable users.
     */
    public boolean isOutboundUserChangeable() ;


    /**
     * Add a dialing strategy listener to this dialing strategy.  The
     * listener will be informed when the dialing process is aborted or
     * initiated.
     */
    public void addDialingStrategyListener(DialingStrategyListener listener) ;


    /**
     * Remove a dialing strategy listener from this dialing strategy object.
     */
    public void removeDialingStrategyListener(DialingStrategyListener listener) ;

    /**
     * gets an array of Dialing Strategy Listeners that were added.
     */
    public DialingStrategyListener[] getDialingStrategyListeners();
}
