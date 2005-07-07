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

package org.sipfoundry.sipxphone.app ;

import org.sipfoundry.stapi.* ;
import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.callcontrol.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;

import org.sipfoundry.sipxphone.sys.app.core.* ;
import org.sipfoundry.sipxphone.app.transfer.* ;
import javax.telephony.* ;
import javax.telephony.callcontrol.* ;

/**
 * 'The' BlindTransferApp Application
 *
 *  THIS CODE IS DEPRECATED -- This code has not been tested in YEARS.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class BlindTransferApp extends Application
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final String RESOURCE_FILE = "transfer.properties" ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    boolean m_bDidCancel = true ;
    PCall   m_callTransferee = null ;
    PCall   m_callMonitor ;
    Object  m_objTimeToExit = new Object() ;


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public void main(String argv[])
    {
        String strSourceCallId = null ;
        String strTargetAddress = null ;

        if ((argv != null) && (argv.length >= 1))
        {
            strSourceCallId = argv[0] ;
        }

        if ((argv != null) && (argv.length >= 2))
        {
            strTargetAddress = argv[1] ;
        }


        m_callMonitor = null  ;
        AppResourceManager.getInstance().addStringResourceFile(RESOURCE_FILE) ;

        m_callTransferee = getTransferee(strSourceCallId) ;
        if (m_callTransferee != null) {

            if ((m_callTransferee.getConnectionState() == PCall.CONN_STATE_CONNECTED) ||
                (m_callTransferee.getConnectionState() == PCall.CONN_STATE_HELD) ||
                (m_callTransferee.getConnectionState() == PCall.CONN_STATE_UNKNOWN)) {

                // Unhook the call from the core world
                Shell.getCallManager().ignoreCall(m_callTransferee) ;
                m_callTransferee.m_bTransferControllerRelated = true ;

                DialingStrategy strategy = new icTransferDialerStrategy() ;
                if (strTargetAddress == null)
                {
                    // Install our dialing strategy and invoke the dialer
                    strategy.addDialingStrategyListener(new icDialingStrategyListener()) ;
                    Shell.getInstance().setDialingStrategy(strategy, m_callTransferee) ;
                    Shell.getInstance().dial() ;
                }
                else
                {
                    try
                    {
                        strategy.dial(PAddressFactory.getInstance().createAddress(strTargetAddress), m_callTransferee)  ;
                    }
                    catch (Exception e)
                    {
                        Shell.getInstance().showUnhandledException(e, false) ;
                    }
                }

                try
                {
                    synchronized (m_objTimeToExit)
                    {
                        m_objTimeToExit.wait() ;
                    }
                }
                catch (InterruptedException ie) { /* burp */ }
            }
        }

        // Clean up our resources
        AppResourceManager.getInstance().
                removeStringResourceFile(RESOURCE_FILE);
    }


    public Application getApplication()
    {
        return this ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Get the transferee for the blind transfer.  If a strCallId parameter
     * is supplied then that call will be used for the transfer.  If no
     * call-id is supplied, then the code will pick the call in focus or a
     * held call to focus.  If more than one held call is available, the user
     * is prompted.
     */
    protected PCall getTransferee(String strCallId)
    {
        PCall callTransferee = null ;

        if (strCallId != null)
        {
            callTransferee = Shell.getCallManager().getCallByCallID(strCallId) ;
        }
        else
        {
            callTransferee = Shell.getCallManager().getInFocusCall() ;

            // Try to pull a call off hold if we don't have an infocus call
            if (callTransferee == null) {
                PCall heldCalls[] = Shell.getCallManager().getCallsByState(PCall.CONN_STATE_HELD) ;

                if (heldCalls.length == 1) {
                    callTransferee = heldCalls[0] ;
                } else if (heldCalls.length > 1) {
                    SelectHeldCallForm formSelectCall = new SelectHeldCallForm(this,
                            AppResourceManager.getInstance().getString("lblSelectHeldCallTitle"),
                            AppResourceManager.getInstance().getString("lblSelectHeldCallAction"),
                            AppResourceManager.getInstance().getString("hint/core/dial_strategy/transfer")) ;

                    if (formSelectCall.showModal() == SelectHeldCallForm.OK)
                    {
                        callTransferee = formSelectCall.getSelectedCall() ;
                    }
                }
            }
        }

        return callTransferee ;
    }

    /**
     * Restore the original call to focus
     */
    protected void restoreOriginalCall(PCall callOriginal)
    {
        // Restore original call
        try {
            callOriginal.m_bTransferControllerRelated = false ;
            callOriginal.m_bTransferControllerOriginalCall = false ;
            callOriginal.m_bTransferControllerMonitorCall = false ;

            Shell.getCallManager().monitorCall(callOriginal) ;
            callOriginal.releaseFromHold() ;
        } catch (PCallException ee) {
            SysLog.log(ee) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////
    /**
     * The Dialing Strategy listener is the guy who receives dialing event and should
     * go ahead and kick off the transfer.
     */
    private class icDialingStrategyListener implements DialingStrategyListener
    {
        public void dialingInitiated(PCall call, PAddress address) {
            Shell.getInstance().setDialingStrategy(null) ;
        }

        public void dialingAborted(PCall call)
        {
            Shell.getInstance().setDialingStrategy(null) ;

            if (m_bDidCancel) {

                m_callTransferee.m_bTransferControllerRelated = false ;
                m_callTransferee.m_bTransferControllerOriginalCall = false;

                Shell.getCallManager().monitorCall(m_callTransferee) ;
            }

            // Notify whoever cares that we are complete
            synchronized (m_objTimeToExit) {
                m_objTimeToExit.notifyAll() ;
            }
        }
    }





    /**
     * Dialer Strategy used when attemping to transfer a call
     */
    private class icTransferDialerStrategy extends AbstractDialingStrategy
    {
        /**
         * 'Dial' or invoke the method
         */
        public void dial(PAddress address)
        {
            PCall call = Shell.getInstance().getDialingStrategyCall() ;
            dial(address, call) ;
        }


        /**
         * Transfer a specific call
         */
        public void dial(PAddress address, PCall call)
        {
            m_bDidCancel = false ;

            if (call != null) {
                // Tell everyone we are about to transfer and disable media
                // and place call on hold
                fireDialingInitiated(call, address) ;
                try
                {
                    Shell.getMediaManager().setAudioDevice(PAudioDevice.ID_NONE) ;
                }
                catch (PMediaException e)
                {
                    SysLog.log(e) ;
                }


                // The original call should be ignored from this point forward.
                Shell.getCallManager().ignoreCall(call) ;
                call.m_bTransferControllerRelated = true ;
                call.m_bTransferControllerOriginalCall = true ;


                // Do the transfer
                try
                {
                    m_callMonitor = call.transfer(address) ;
                    m_callMonitor.m_bTransferControllerMonitorCall = true ;

                    // Ignore the newly created call.
                    Shell.getCallManager().ignoreCall(m_callMonitor) ;
                    m_callMonitor.m_bTransferControllerRelated = true ;
                    m_callMonitor.ignoreExternalDrops(true) ;

                    // Kick off a monitor to watch the transfer
                    BlindTransferMonitor monitor = new BlindTransferMonitor(getApplication(), call, m_callMonitor, m_objTimeToExit) ;
                    monitor.start() ;

                }
                catch (PCallAddressException cae)
                {
                    MessageBox messageBox = new MessageBox(getApplication(), MessageBox.TYPE_ERROR) ;
                    messageBox.setMessage(cae.getMessage()) ;
                    messageBox.showModal() ;


                    restoreOriginalCall(call) ;

                    // Notify whoever cares that we are complete
                    synchronized (m_objTimeToExit)
                    {
                        m_objTimeToExit.notifyAll() ;
                    }

                }
                catch (Exception e)
                {
                    MessageBox messageBox = new MessageBox(getApplication(), MessageBox.TYPE_ERROR) ;
                    messageBox.setMessage(messageBox.getString("lblErrorTextUnhandledException") + e.getMessage()) ;
                    messageBox.showModal() ;

                    // Restore original call
                    restoreOriginalCall(call) ;

                    // Notify whoever cares that we are complete
                    synchronized (m_objTimeToExit)
                    {
                        m_objTimeToExit.notifyAll() ;
                    }
                }
            }
            else
                throw new IllegalArgumentException("Cannot transfer a null call") ;
        }


        /**
         * Get the string based instructions for this strategy.  This text is
         * typically displayed in lower half of the "dialer" form.
         */
        public String getInstructions()
        {
            return AppResourceManager.getInstance().getString("lblTransferInstructions") ;
        }


        /**
         * Gets the string based representation of this strategy.  This text is
         * typically prepended the the form name.
         */
        public String getFunction()
        {
            return AppResourceManager.getInstance().getString("lblTransferStategyFunction") ;
        }


        /**
         * Gets the string based representation of the dialing strategy action.
         * This text is typically displayed as the B3 button label
         */
        public String getAction()
        {
            return AppResourceManager.getInstance().getString("lblTransferStategyAction") ;
        }


        /**
         * Gets the hint text associatd with the dialing strategy action.  This
         * hint text is typically displayed when the B3 buttons is pressed and
         * held down.
         */
        public String getActionHint()
        {
            return AppResourceManager.getInstance().getString("hint/core/dial_strategy/transfer") ;
        }

        /**
         * Get the cancel status for this dialing strategy.
         *
         * @return boolean true if this operation/strategy can be canceled,
         *         otherwise false
         */
        public boolean isCancelable()
        {
            return true ;
        }
    }
}
