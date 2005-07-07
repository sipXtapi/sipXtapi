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
import java.util.* ;

/**
 * This Abstract Dialing Strategy adds some of the utility implementation
 *
 * @author Robert J. Andreasen, Jr.
 */
public abstract class  AbstractDialingStrategy implements DialingStrategy
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Vector m_vListeners ;


    /**
     * Aborting the dialing process; this implementation invokes
     * fireDialingAbort to notify listeners of the abortion.
     */
    public void abort(PCall call)
    {
        fireDialingAbort(call) ;
    }


    /**
     * Abort the dialing process; this implementation determines by the target
     * call by looking at the dialing strategy call or the infocus call and
     * then invokes abort(call).
     */
    public void abort()
    {
        Shell shell = Shell.getInstance() ;

        PCall call = shell.getDialingStrategyCall() ;
        if (call == null) {
            call = shell.getCallManager().getInFocusCall() ;
        }
        abort(call) ;
    }

     /**
     * Dial this remote address with the specified local address or
     * invoke the strategic method/action on a specific call.
     *
     * <p>In this abstract class implementation, this is the same as calling
     * <i>dial( remoteAddress, call )</i>. Users should override this if they want
     * different behaviour based on localAddress.
     */
     public void dial(PAddress localAddress, PAddress remoteAddress, PCall call)
     {
        dial(remoteAddress, call);
     }

     /**
     * Dial this remote address with the specified local address or
     * invoke the strategic method/action.
     *
     * <p>In this abstract class implementation, this is the same as calling
     * <i>dial( remoteAddress)</i>. Users should override this if they want
     * different behaviour based on localAddress.
     */
     public void dial(PAddress localAddress, PAddress remoteAddress)
    {
        dial(remoteAddress);
    }


    /**
     * Evaluates if this stategy supports a changeable user.  For example, a
     * new call can be placed as any valid user, however, a specific call leg
     * (as in conference), cannot.  Similary, once could not transfer a call
     * as another user.
     *
     * @return boolean true if this dialing stragegy supports changeable users.
     */
    public boolean isOutboundUserChangeable()
    {
        // By default, the user is not changeable.
        return false ;
    }


    /**
     * Add a dialing strategy listener to this dialing strategy.  The
     * listener will be informed when the dialing process is aborted or
     * initiated.
     */
    public void addDialingStrategyListener(DialingStrategyListener listener)
    {
        if (m_vListeners == null)
            m_vListeners = new Vector() ;

        if (!m_vListeners.contains(listener)) {
            m_vListeners.addElement(listener) ;
        }
    }


    /**
     * Remove a dialing strategy listener from this dialing strategy object.
     */
    public void removeDialingStrategyListener(DialingStrategyListener listener)
    {
        if (m_vListeners != null) {
            m_vListeners.removeElement(listener) ;
        }
    }

    /**
     * gets an array of Dialing Strategy Listeners that were added.
     * If none were added, it will return an empty array.
     */
    public DialingStrategyListener[] getDialingStrategyListeners(){
        if (m_vListeners == null){
            m_vListeners = new Vector() ;
        }
        int iSize = m_vListeners.size();
        DialingStrategyListener[] arrayListeners  =
            new DialingStrategyListener[iSize];
        for( int i = 0; i<iSize; i++ ){
            arrayListeners[i] = (DialingStrategyListener)
                (m_vListeners.elementAt(i)) ;
        }
        return arrayListeners;
    }

    protected void fireDialingAbort(PCall call)
    {
        if (m_vListeners != null) {
            // Tell everyone in the category about the event
            for (Enumeration e = m_vListeners.elements(); e.hasMoreElements();) {

                DialingStrategyListener listener = (DialingStrategyListener) e.nextElement() ;
                if (listener != null) {
                    try {
                        listener.dialingAborted(call) ;
                    } catch (Throwable t) {
                        Shell.getInstance().showUnhandledException(t, true) ;
                    }
                }
            }
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    protected void fireDialingInitiated(PCall call, PAddress address)
    {
        if (m_vListeners != null) {
            // Tell everyone in the category about the event
            for (Enumeration e = m_vListeners.elements(); e.hasMoreElements();) {
                DialingStrategyListener listener = (DialingStrategyListener) e.nextElement() ;
                if (listener != null) {
                    try {
                        listener.dialingInitiated(call, address) ;
                    } catch (Throwable t) {
                        Shell.getInstance().showUnhandledException(t, true) ;
                    }
                }
            }
        }
    }
}
