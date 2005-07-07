/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/ConsultativeTransferApp.java#2 $
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

import java.net.* ;
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
import org.sipfoundry.sipxphone.service.Timer ;
import org.sipfoundry.sipxphone.sys.app.ShellApp ;
import org.sipfoundry.sipxphone.service.Logger ;


import javax.telephony.* ;
import javax.telephony.callcontrol.* ;


/**
 * This xpression drives the consultative transfer sequence.
 * Unfortunately, transfer is complex and has an large number of failure
 * conditions.  Worse, mapping consultative transfer through JTAPI has
 * proven to be difficult.
 * <p>
 * So here is the basic user interaction:
 *
 * Joe calls sally, sally decides to transfer Joe to Bill.  So, Sally
 * presses the "Transfer" button, keys in Bill's extension, and either does
 * one of two things: 1) hangs up before Bill answers, thereby switching to
 * blind transfer or 2) consults with Bill and then hangs up or presses
 * transfer to complete the consultative transfer (sometimes called
 * attended transfer). So, terminology wise, Joe is the transferee, Sally
 * is the transfer controller, and Bill is the transfer target.
 * <p>
 * From a signalling perspective, here is a simplified look at SIP message
 * flow for the consultative-turned-blind transfer and for the pure
 * consultative transfer:
 * <pre>
 * Consultative-Turned-Blind:
 *
 * Joe                   Sally                  Bill
 * Transferee            Transfer Controller    Transfer Target
 *
 *     ------INVITE------->
 *    <------180----------
 *    <------200 OK-------
 *     ------ACK---------->
 *                          -------INVITE--------->
 *                         <-------180------------
 *                          -------CANCEL--------->
 *                         <-------200 OK---------
 *    <------REFER---------
 *     ------202----------->
 *     ---------------------INVITE---------------->
 *    <---------------------180-------------------
 *    <---------------------200 OK----------------
 *     ---------------------ACK------------------->
 *     ------NOTIFY-------->
 *    <------200 OK--------
 *
 * Consultative:
 *
 *     ------INVITE------->
 *    <------180----------
 *    <------200 OK-------
 *     ------ACK---------->
 *                          -------INVITE--------->
 *                         <-------180------------
 *                         <-------200 OK---------
 *                          -------ACK------------>
 *    <------REFER---------
 *     ------202----------->
 *     ---------------------INVITE w/ REPLACES---->
 *    <---------------------200 OK----------------
 *     ------NOTIFY-------->
 *    <------200 OK--------
 *    <------BYE-----------
 *     ------200 OK-------->
 *                           -------BYE------------>
 *                          <-------200 OK---------
 * </pre>
 * From a JTAPI perspective, this is accomplished by issuing the following
 * pseudocode:
 *
 * <code>
 * callTransferTarget = new Call()
 * callTransferTarget.consult(callTransferee, addressTarget)
 *
 * Consultative-Turned-Blind:
 * callTransferTarget.drop() ;
 * callTransferee.placeOnHold()
 * Call callMonitor = callTransferee.transfer(addressTarget) ;
 *
 * Consultative:
 * callTransferTarget.transfer(callTransferee) ;
 * </code>
 * <p>
 * The problems arise when you consider all of the possible failure
 * conditions, that people can hang up at any point, or that messages can
 * be lost, etc.  For example, consider the race in the
 * consultative-turn-blind case, where Bill picks up the instant that Sally
 * hangs up.  I believe the underlying SIP stack does the correct things,
 * however this external event is generally transparent to the application
 * and the event stream is somewhat unpredictable -- or hard to derive what
 * happened.  For example, did that connection drop because the transfer
 * failed or because the user hung up?
 * <p>
 * To ease the complexity of the problem, I introduced a layer of
 * abstraction (TransferControllerListener) and tried to convert all of the
 * possible failure conditions into a series of events:
 * <ul>
 * <li>  consultSuceeded
 * <li>  consultFailed
 * <li>  consultAborted
 * <li>  transferSucceeded
 * <li>  transferFailed
 * <li>  transferAborted
 * <li>  transfereeHungup
 * <li>  transferTargetHungup
 * </ul>
 * <p>
 * The Transfer application keys off these events and gives error messages,
 * recovers parties, etc.  In the case of consultative-turns-blind, control
 * is handed off to a less elegant (and legacy) solution using the
 * BlindTransferMonitor.
 * <p>
 * In the code below, you will also see two different DialingStrategy
 * implementations.  Dialing Strategies are used to extend/change the
 * default behavior or dialing.  For example, in the consultative case, we
 * need issue a call.consult(...) instead of a call.connect(...).  The code
 * installs a dialing strategy for setting up the consultative call and one
 * for completing the transfer.
 * <p>
 * Unfortunately, the code also has a number of boolean "we did this" flags
 * to help with error reporting and decision making.  If I were to rewrite
 * this, I would probably introduce a state table and use that to track
 * where we were in the process instead of the various switches.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ConsultativeTransferApp extends Application
    implements TransferControllerListener
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final String RESOURCE_FILE = "transfer.properties" ;
    private static boolean DEBUG_TRANSFER = false ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    boolean  m_bDidCancel = true ;              // Did we cancel the operation?
    PCall    m_callTransferee = null ;          // Ref to transferee call
    PCall    m_callTransferTarget = null ;      // Ref to transfer target call
    Object   m_objTimeToExit = new Object() ;   // Object to signal completion
    boolean  m_bHasTransferred ;                // Transfer kicked off?
    boolean  m_bHasBlindTransferred ;           // Blind Transfer kicked off
    boolean  m_bHasConsulted = false;           // Consult kicked off?
    PAddress m_addressTransferTarget ;          // What is the target address?
    boolean  m_bTransfereeHungup ;               // Did the transferee drop?
    boolean  m_bReportedFailure ;               // Did we report that the transfer failed?
    boolean  m_bDroppingTransferTarget = false; //for locking access to drop method. It's recursive
    boolean  m_bDroppingCallTransferee = false; //for locking access to drop method. It's recursive
    icConsultDialerStrategy m_strategy = null ; // The transfer strategy
    ConsultativeTransferMonitor m_monitor = null ; // Monitors the consultative transfer


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Transfer application entry point- coordinates the transfer
     */
    public void main(String argv[])
    {
        AppResourceManager.getInstance().addStringResourceFile(RESOURCE_FILE) ;

        m_bHasTransferred = false ;
        m_bHasBlindTransferred = false ;
        m_bTransfereeHungup = false ;
        m_bReportedFailure = false ;

        m_callTransferee = Shell.getCallManager().getInFocusCall() ;

        // Try to pull a call off hold if we don't have an infocus call
        if (m_callTransferee == null) {
            PCall heldCalls[] = Shell.getCallManager().getCallsByState(PCall.CONN_STATE_HELD) ;

            if (heldCalls.length == 1) {
                m_callTransferee = heldCalls[0] ;
            } else if (heldCalls.length > 1) {
                SelectHeldCallForm formSelectCall = new SelectHeldCallForm(this,
                        AppResourceManager.getInstance().getString("lblSelectHeldCallTitle"),
                        AppResourceManager.getInstance().getString("lblSelectHeldCallAction"),
                        AppResourceManager.getInstance().getString("hint/core/dial_strategy/transfer")) ;
                if (formSelectCall.showModal() == SelectHeldCallForm.OK) {
                    m_callTransferee = formSelectCall.getSelectedCall() ;
                }
            }
        }


        // It is possible that we do not have any calls available or the end
        // user hit the cancel button in the select held calls form.  For now,
        // we simply exiting gracefully and without error.  Perhaps, we should
        // tell the user that there were no calls to transfer?  You would hope
        // the user is smart enough to figure that out.
        if (m_callTransferee != null) {

            if ((m_callTransferee.getConnectionState() == PCall.CONN_STATE_CONNECTED) ||
                (m_callTransferee.getConnectionState() == PCall.CONN_STATE_HELD) ||
                (m_callTransferee.getConnectionState() == PCall.CONN_STATE_UNKNOWN)) {


                // Create the consulative call
                try {
                    m_callTransferTarget = PCall.createCall() ;
                }
                catch (Exception e)
                {
                    Shell.getInstance().showUnhandledException(e, false) ;
                }

                // Create the monitor
                m_monitor = new ConsultativeTransferMonitor(m_callTransferee,
                        m_callTransferTarget) ;
                PAddress addresses[] = m_callTransferee.getParticipants() ;
                if (addresses.length > 0)
                {
                    m_monitor.setTransfereeAddress(addresses[0]) ;
                }

                // Install our dialing strategy and invoke the dialer
                m_strategy = new icConsultDialerStrategy() ;
                m_strategy.addDialingStrategyListener(new icDialingStrategyListener(m_monitor)) ;
                Shell.getInstance().setDialingStrategy(m_strategy, m_callTransferTarget) ;

                // Add ourselvies as a listener
                m_monitor.addTransferListener(this) ;

                // Kick off the consult
                Shell.getInstance().dial() ;

                try {
                    synchronized (m_objTimeToExit)
                    {
                        m_objTimeToExit.wait() ;
                    }
                } catch (InterruptedException ie) { /* burp */ }


                // Clean up the consultative transfer monitor
                if (m_monitor != null)
                {
                    m_monitor.removeTransferListener(this);
                    m_monitor.releaseListeners();
                    m_monitor = null ;
                }
            }
        }

        // Clean up our resources
        AppResourceManager.getInstance().removeStringResourceFile(RESOURCE_FILE) ;
        
        System.out.println("*** TRANSFER APP EXITING ****") ;
    }


    /**
     * Invoked if the transfer controller successfully consults with the
     * transfer target.
     */
    public void consultSucceeded(TransferControllerEvent event)
    {
        if (Logger.isEnabled())
        {
            Logger.post("consultative_transfer",
                    Logger.TRAIL_NOTIFICATION,
                    "consultSucceeded",
                    event.getTargetAddress(),
                    event.getTransfereeCall().getCallID()) ;
        }
    }


    /**
     * Invoked if the transfer controller fails to consult with the transfer
     * target
     */
    public void consultFailed(TransferControllerEvent event)
    {
        if (!m_bReportedFailure)
        {
            m_bReportedFailure = true ;

            if (Logger.isEnabled())
            {
                Logger.post("consultative_transfer",
                        Logger.TRAIL_NOTIFICATION,
                        "consultFailed",
                        event.getTargetAddress(),
                        event.getTransfereeCall().getCallID()) ;
            }

            new icReportTransferFailureAndRecover().start() ;
        }
    }



    /**
     * Invoked if the transfer controller hangs up during the consult period
     * before the transfer target answers the call
     */
    public void consultAborted(TransferControllerEvent event)
    {
        if (Logger.isEnabled())
        {
            Logger.post("consultative_transfer",
                    Logger.TRAIL_NOTIFICATION,
                    "consultAborted",
                    event.getTargetAddress(),
                    event.getTransfereeCall().getCallID()) ;
        }

        recoverTransferee() ;
    }



    /**
     * Invoked if the transfer has been completed and all parties have been
     * successfully transferred to the intended target.
     */
    public void transferSucceeded(TransferControllerEvent event)
    {
        if (Logger.isEnabled())
        {
            Logger.post("consultative_transfer",
                    Logger.TRAIL_NOTIFICATION,
                    "transferSucceeded",
                    event.getTargetAddress(),
                    event.getTransfereeCall().getCallID()) ;
        }

        cleanupCalls() ;
    }


    /**
     * Invoked if the transfer fails for any reason, excluding the transfer
     * controller aborting.
     */
    public void transferFailed(TransferControllerEvent event)
    {
        if (!m_bReportedFailure)
        {
            m_bReportedFailure = true ;

            if (Logger.isEnabled())
            {
                Logger.post("consultative_transfer",
                        Logger.TRAIL_NOTIFICATION,
                        "transferFailed",
                        event.getTargetAddress(),
                        event.getTransfereeCall().getCallID()) ;
            }

            new icReportTransferFailureAndRecover().start() ;
        }
    }


    /**
     * Invoked if the transfer is aborted by the transfer controller.
     */
    public void transferAborted(TransferControllerEvent event)
    {
        if (Logger.isEnabled())
        {
            Logger.post("consultative_transfer",
                    Logger.TRAIL_NOTIFICATION,
                    "transferAborted",
                    event.getTargetAddress(),
                    event.getTransfereeCall().getCallID()) ;
        }

        recoverTransferee() ;
    }


    /**
     * Invoked if the transferee hangs up in the middle of a transfer attempt.
     */
    public void transfereeHungup(TransferControllerEvent event)
    {
        // Optimization: Don't bother if we are dropping it...
        if (m_bDroppingCallTransferee)
            return ;

        m_bTransfereeHungup = true ;

        if ((!m_bHasTransferred) && (!m_bHasBlindTransferred))
        {
            // If the transfer target is still around, then report this to
            // the user, otherwise, we are in an odd siutation and simply
            // clean up.
            if (m_callTransferTarget != null)
            {
				m_callTransferTarget.setAutoDrop(true) ;
                m_callTransferTarget.setHangupHandler(null) ;                

                if (Logger.isEnabled())
                {
                    Logger.post("consultative_transfer",
                            Logger.TRAIL_NOTIFICATION,
                            "transfereeHungup",
                            event.getTargetAddress(),
                            event.getTransfereeCall().getCallID()) ;
                }

                new icReportTransfereeHungupAndRecover().start() ;
            }
            else
            {
                cleanupCalls() ;
            }
        }
        else
        {
            cleanupCalls() ;
        }
    }


    /**
     * Invoked whenever the transfer target drops.  We only really care about
     * this in a few circumstances:
     *
     * - We're consulting with the target (should report it)
     * - We have already transferred (assume the transfer worked, cleanup)
     * - We have initiated a blind transfer (we should drop it)
     *
     */
    public void transferTargetHungup(TransferControllerEvent event)
    {
        // Optimization: Don't bother if we are dropping...
        if (m_bDroppingTransferTarget)
            return ;

        if (m_bHasBlindTransferred)
        {
            /* - We have initiated a blind transfer (we should drop it) */
            dropTransferTarget() ;
        }
        else if (!m_bHasTransferred)
        {
            /* - We're consulting with the target (should report it) */

            // Make sure we have a transferee, before
            if ((!m_bTransfereeHungup) && (m_callTransferee != null))
            {
                if (Logger.isEnabled())
                {
                    Logger.post("consultative_transfer",
                            Logger.TRAIL_NOTIFICATION,
                            "transferTargetHungup",
                            event.getTargetAddress(),
                            event.getTransfereeCall().getCallID()) ;
                }
                new icReportTargetHungupAndRecover().start() ;
            }
            else
            {
                cleanupCalls() ;
            }
        }
        else
        {
            /* - We have already transferred (assume the transfer worked, cleanup) */
            cleanupCalls() ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /*
     * Allows inner classes to get a reference to the outer class without
     * using funky syntax.
     */
    public ConsultativeTransferApp getApp()
    {
        return this ;
    }


    public void doBlindTransfer(PCall call, PAddress addressTarget)
        throws PSTAPIException, MalformedURLException
    {
        PAddress addressTransferee = null ;

        debugln("doBlindTransfer:") ;
        debugln("  call=" + call) ;
        debugln("  addr=" + addressTarget);

        // The original call should be ignored from this point forward.
        Shell.getCallManager().ignoreCall(call) ;
        Shell.getCallManager().hideCall(call) ;
        call.placeOnHold(null) ;

        // Figure out the transferee
        PAddress participants[] = call.getParticipants() ;
        if (participants.length > 0)
        {
            addressTransferee = participants[0] ;
        }

        // Do the transfer
        Shell.getMediaManager().setAudioDevice(PAudioDevice.ID_NONE) ;
        PCall callMonitor = call.transfer(addressTarget) ;
        if (callMonitor != null)
        {
            Shell.getCallManager().ignoreCall(callMonitor) ;
            Shell.getCallManager().hideCall(callMonitor) ;
            callMonitor.ignoreExternalDrops(true) ;

            // Kick off a monitor to watch the transfer
            BlindTransferMonitor monitor = new BlindTransferMonitor(this, call, callMonitor, m_objTimeToExit) ;
            monitor.start() ;
        }
        else
        {
            TransferUtils.unmarkBlindTransferCall(call) ;
            TransferUtils.reportTransferFailure(this) ;
            TransferUtils.recoverBlindTransferee(call) ;
        }
    }



    /**
     * Report that the transfer has failed report the ability to recover the
     * original call.
     */
    protected void reportTransferFailure()
    {
        debugln("reportTransferFailure") ;

        MessageBox msgBox = new MessageBox(this, MessageBox.TYPE_INFORMATIONAL) ;

        msgBox.setTitle("Transfer Failed") ;
        msgBox.setMessage("Your transfer cannot be completed. Please press Ok to resume your original call.") ;

        msgBox.showModal(true) ;
    }


    /**
     * Report that the transferee has hungup.
     */
    protected void reportTransfereeHungup()
    {
        debugln("reportTransfereeHungup") ;

        MessageBox msgBox = new MessageBox(this, MessageBox.TYPE_INFORMATIONAL) ;

        msgBox.setTitle("Transfer Status") ;
        msgBox.setMessage("Transfer cannot be completed: the party you were " +
        "transferring has disconnected. Press Ok to continue your call or hang up.") ;
        msgBox.showModal(true) ;
    }


    /**
     * Report that the transfer target has hung up.
     */
    protected void reportTransferTargetHungup()
    {
        debugln("reportTransferTargetHungup") ;

        MessageBox msgBox = new MessageBox(this, MessageBox.TYPE_INFORMATIONAL) ;

        msgBox.setTitle("Transfer Status") ;
        msgBox.setMessage("Transfer cannot be completed. The destination call has been disconnected. Press Ok to resume your original call.") ;

        msgBox.showModal(true) ;
    }




    /**
     * Drops the transferee and assumes a regular conversation with the
     * transfer target.
     */
    protected void recoverTransferTarget()
    {
        debugln("recoverTransferTarget") ;

        // Kill the transferee
        dropTransferee() ;

        // If the target has disconnected, drop the call, otherwise
        // release it from hold
        PCall callTransferTarget = m_callTransferTarget ;
		m_callTransferTarget = null ;        
        if (callTransferTarget != null)
        {
            int iConnState = callTransferTarget.getConnectionState() ;
            if (    (iConnState != PCall.CONN_STATE_DISCONNECTED) &&
                    (iConnState != PCall.CONN_STATE_FAILED))
            {
                try
                {
                    if (Logger.isEnabled())
                    {
                        Logger.post("consultative_transfer",
                                Logger.TRAIL_NOTIFICATION,
                                "recoverTransferTarget",
								callTransferTarget.getCallID(),
                                null) ;
                    }

					callTransferTarget.setAutoDrop(true) ;
					callTransferTarget.setHangupHandler(null) ;                    
					callTransferTarget.releaseFromHold() ;
					
					Shell.getCallManager().showCall(callTransferTarget) ;
                    Shell.getCallManager().monitorCall(callTransferTarget) ;
                }
                catch (Exception e)
                {
                    Shell.getInstance().showUnhandledException(e, false) ;
                }                
            }
            else
            {
                dropTransferTarget() ;
            }
        }

        // Signal the transfer application to exit
        synchronized (m_objTimeToExit)
        {
            m_objTimeToExit.notifyAll() ;
        }
    }


    /**
     * Drop the transfer target call and recover the transferee if possible
     */
    protected void recoverTransferee()
    {
        debugln("recoverTransferee") ;

        // Drop the transfer target
        dropTransferTarget() ;

        // If the transferee has disconnected, drop the call, otherwise
        // release it from hold
        PCall callTransferee = m_callTransferee ;
		m_callTransferee = null ; 
        if (callTransferee != null)
        {
            int iConnState = callTransferee.getConnectionState() ;
            if (    (iConnState == PCall.CONN_STATE_CONNECTED) ||
                    (iConnState == PCall.CONN_STATE_HELD))
            {
                try
                {
                    if (Logger.isEnabled())
                    {
                        Logger.post("consultative_transfer",
                                Logger.TRAIL_NOTIFICATION,
                                "recoverTransferee",
                                callTransferee.getCallID(),
                                null) ;
                    }
					
					callTransferee.setAutoDrop(true) ;
                    callTransferee.setHangupHandler(null) ;                    
                    callTransferee.releaseFromHold() ;

                    Shell.getCallManager().showCall(callTransferee) ;
					Shell.getCallManager().monitorCall(callTransferee) ;   
                }
                catch (Exception e)
                {
                    Shell.getInstance().showUnhandledException(e, false) ;
                }                
            }
            else
            {
                // Kill the transferee
                dropTransferee() ;
            }
        }



        // Signal the transfer application to exit
        synchronized (m_objTimeToExit)
        {
            m_objTimeToExit.notifyAll() ;
        }
    }

    protected synchronized void dropTransferTarget()
    {			
		debugln("dropTransferTarget") ;
		    	
    	PCall callTransferTarget = m_callTransferTarget ;
		m_callTransferTarget = null;
        
        // Drop the transfer target
        if (!m_bDroppingTransferTarget && callTransferTarget != null)
        {
            m_bDroppingTransferTarget = true;
           // Abort the dialing strategy if it is still around
            if (m_strategy != null)
            {
                m_bDidCancel = false ;
                m_strategy.abort() ;
                m_strategy = null ;
            }

            // Drop the call
            try
            {
                if (Logger.isEnabled())
                {
                    Logger.post("consultative_transfer",
                            Logger.TRAIL_NOTIFICATION,
                            "dropTransferTarget",
                            callTransferTarget.getCallID(),
                            null) ;
                }

				callTransferTarget.setAutoDrop(true) ;
                callTransferTarget.setHangupHandler(null) ;
                debugln("---- Dropping target call") ;
                
                Shell.getCallManager().hideCall(callTransferTarget) ;
                Shell.getCallManager().ignoreCall(callTransferTarget) ;
				callTransferTarget.disconnect() ;
            }
            catch (Exception e)
            {
                Shell.getInstance().showUnhandledException(e, false) ;
            }
            
            m_bDroppingTransferTarget = false;
        }
    }


    protected synchronized void dropTransferee()
    {
        debugln("dropTransferee") ;

        // Drop the transferee
		PCall callTransferee = m_callTransferee ;
		m_callTransferee = null ;
		        
        if (!m_bDroppingCallTransferee && callTransferee != null)
        {
            m_bDroppingCallTransferee = true;
            try
            {
                if (Logger.isEnabled())
                {
                    Logger.post("consultative_transfer",
                            Logger.TRAIL_NOTIFICATION,
                            "dropTransferee",
							callTransferee.getCallID(),
                            null) ;
                }

				callTransferee.setAutoDrop(true) ;
				callTransferee.setHangupHandler(null) ;				
				
                debugln("---- Dropping transferee call") ;
				Shell.getCallManager().hideCall(callTransferee) ;
                Shell.getCallManager().ignoreCall(callTransferee) ;                				
				callTransferee.disconnect() ;
            }
            catch (Exception e)
            {
                Shell.getInstance().showUnhandledException(e, false) ;
            }           
            m_bDroppingCallTransferee = false;

        }
    }


    protected void cleanupCalls()
    {
        debugln("cleanupCalls") ;

        dropTransferTarget() ;
        dropTransferee() ;

        // Signal the transfer application to exit
        synchronized (m_objTimeToExit)
        {
            m_objTimeToExit.notifyAll() ;
        }
    }


    protected void debugln(String string)
    {
        if (DEBUG_TRANSFER)
        {
            System.out.println("CTA: " + string) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////

    /**
     * Provides a thread context for reporting a failure and then recovering
     */
    private class icReportTransferFailureAndRecover extends Thread
    {
        public void run()
        {        	
            reportTransferFailure() ;
            recoverTransferee() ;
        }
    }


    /**
     * Provides a thread context for reporting a failure and then recovering
     */
    private class icReportTransfereeHungupAndRecover extends Thread
    {
        public void run()
        {
            reportTransfereeHungup() ;

            if (m_bHasConsulted)
            {                        	
                recoverTransferTarget() ;
            }
            else
            {            
                cleanupCalls() ;
            }
        }
    }


    /**
     * Provides a thread context for reporting a failure and then recovering
     */
    private class icReportTargetHungupAndRecover extends Thread
    {
        public void run()
        {
			// Drop the transfer target
			dropTransferTarget() ;
        	        	
            reportTransferTargetHungup() ;
            recoverTransferee() ;
        }
    }



    /**
     * Provides a thread context for recovering
     */
    private class icRecoverTransferee extends Thread
    {
        public void run()
        {
            recoverTransferee() ;
        }
    }



    /**s
     * The Dialing Strategy listener is the guy who receives dialing event and should
     * go ahead and kick off the transfer.
     */
    private class icDialingStrategyListener implements DialingStrategyListener
    {
        ConsultativeTransferMonitor m_monitor ;

        icDialingStrategyListener(ConsultativeTransferMonitor monitor)
        {
            m_monitor = monitor ;
        }


        public void dialingInitiated(PCall call, PAddress address)
        {
            m_strategy = null ;
            Shell.getInstance().setDialingStrategy(null) ;
        }

        public void dialingAborted(PCall call)
        {
            m_strategy = null ;
            Shell.getInstance().setDialingStrategy(null) ;

            m_monitor.fireConsultAborted() ;

            /*
            if ((m_bDidCancel) &&
                    (m_callTransferee != null) &&
                    (m_callTransferee.getConnectionState() != PCall.CONN_STATE_DISCONNECTED))
            {
                Shell.getCallManager().monitorCall(m_callTransferee) ;

                // Notify whoever cares that we are complete
                synchronized (m_objTimeToExit)
                {
                    m_objTimeToExit.notifyAll() ;
                }
            }
            */
        }
    }


    /**
     * Provides a default action for when the users hangs up a call.  It is
     * assumed that 'hanging up' should complete the transfer.
     */
    private class icTransferHangupActionHandler implements ActionHandler
    {
        public boolean performAction(String strAction, Object objParam1, Object objParam2)
        {
            boolean bRC = false ;
            PCall call = (PCall) objParam1 ;
            boolean bDroppedTargetCall = false ;
            PCall   callTransferTarget = m_callTransferTarget ;
			PCall   callTransferee = m_callTransferee ; 


			/*
			 * We are switching to blind transfer, put on your seat belt, 
			 * kansas is going bye bye. 
			 */
            if ((call != null) && (callTransferee != null) && (callTransferTarget != null))
            {
                if (strAction.equalsIgnoreCase("ACTION_HANGUP"))
                {
                    try
                    {
                        
                        PtCallControlCall callTransfereeJTAPI = (PtCallControlCall) callTransferee.getJTAPICall() ;
						PtCallControlCall callTargetJTAPI = (PtCallControlCall) callTransferTarget.getJTAPICall() ;

                        // If we are still ringing the target then hangup and then
                        // complete transfer.
                        int iTargetState = callTransferTarget.getConnectionState() ;
                        if (    (iTargetState == PCall.CONN_STATE_TRYING) ||
                                (iTargetState == PCall.CONN_STATE_OUTBOUND_ALERTING))
                        {
                            // Hang up / abort connection attempts
                            PAddress participants[] = callTransferTarget.getParticipants() ;
                            for (int i=0; i<participants.length; i++)
                            {
                                // m_callTransferTarget.disconnect(participants[i]) ;
                                bDroppedTargetCall = true ;

                            }
                        }

                        Shell.getCallManager().hideCall(callTransferTarget) ;
                        Shell.getCallManager().hideCall(callTransferee) ;

                        callTransferee.setHangupHandler(null) ;
                        callTransferTarget.setHangupHandler(null) ;

                        if (bDroppedTargetCall)
                        {
                            // If we have already blind transferred, make sure we
                            // kick out.  I don't that this should ever happen,
                            // but it is a plausible risk.
                            if (m_bHasBlindTransferred)
                                return true ;

                            // If we have dropped the target call, switch to
                            // blind transfer
                            if (Logger.isEnabled())
                            {
                                Logger.post("blind_transfer",
                                        Logger.TRAIL_NOTIFICATION,
                                        "transfer",
                                        callTransferee.getCallID(),
                                        m_addressTransferTarget) ;
                            }

                            // Clean up the consultative transfer monitor
                            if (m_monitor != null)
                            {
                                m_monitor.removeTransferListener(getApp());
                                m_monitor.releaseListeners();
                                m_monitor = null ;
                            }

                            dropTransferTarget() ;
                            m_bHasBlindTransferred = true ;                            
                            doBlindTransfer(callTransferee, m_addressTransferTarget) ;
                        }
                        else
                        {
                            if (Logger.isEnabled())
                            {
                                Logger.post("consultative_transfer",
                                        Logger.TRAIL_NOTIFICATION,
                                        "transfer",
                                        m_callTransferTarget.getCallID(),
                                        m_callTransferTarget.getCallID()) ;
                            }
                            m_bHasTransferred = true ;
                            callTransferTarget.transfer(callTransferee) ;
                        }
                    }
                    catch (Exception e)
                    {
                        Shell.getInstance().showUnhandledException(e, false) ;
                    }
                    bRC = true ;
                }
                else    // ACTION_ABORT
                {
                    new icRecoverTransferee().start() ;
                    bRC = true ;
                }
            }
            return bRC ;
        }


        public String getLabel()
        {
            return "Transfer" ;
        }


        public String getHint()
        {
            return "Transfer|Completes the transfer now." ;
        }
    }


    /**
     * Dialer Strategy used when attemping to transfer a call
     */
    private class icConsultDialerStrategy extends AbstractDialingStrategy
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
         * Begin Consult with a remote party
         */
        public void dial(PAddress address, PCall call)
        {
            boolean bError = true ;
            m_bDidCancel = false ;
            m_addressTransferTarget = address ;
			PCall   callTransferTarget = m_callTransferTarget ;
			PCall   callTransferee = m_callTransferee ; 
            
            if ((call != null) && (callTransferee != null) && (callTransferTarget != null))            
            {

                // Tell everyone we are about to transfer and disable media
                // and place call on hold
                fireDialingInitiated(call, address) ;

                // Hide the original call
                Shell.getCallManager().hideCall(callTransferee) ;

                // Do not auto-drop any of these calls.
				callTransferee.setAutoDrop(false) ;
				callTransferTarget.setAutoDrop(false) ;

                // Do the Consult
                try
                {

                    call.setHangupHandler(new icTransferHangupActionHandler()) ;
					callTransferTarget.consult(callTransferee, address) ;
                    m_bHasConsulted = true ;

                    bError = false ;

                    if (Logger.isEnabled())
                    {
                        Logger.post("consultative_transfer",
                                Logger.TRAIL_NOTIFICATION,
                                "consult",
                                address,
                                null) ;
                    }

                }
                catch (PCallAddressException cae)
                {
                    call.setHangupHandler(null) ;

                    MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                    messageBox.setMessage(cae.getMessage()) ;
                    messageBox.showModal() ;

                }
                catch (Exception e)
                {
                    call.setHangupHandler(null) ;

                    MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                    messageBox.setMessage(messageBox.getString("lblErrorTextUnhandledException") + e.getMessage()) ;
                    messageBox.showModal() ;
                }

                if (bError)
                {
                    // Restore Original Call
                    try
                    {
                        Shell.getCallManager().showCall(callTransferee) ;
                        callTransferee.releaseFromHold() ;
                        try
                        {
                            Thread.sleep(1000);
                        }
                        catch (Exception e)
                        {
                            System.out.println(e) ;
                        }

                        callTransferee.setAutoDrop(true) ;
                    }
                    catch (PCallException ee)
                    {
                        Shell.getInstance().showUnhandledException(ee, false) ;
                    }
                    
                    m_callTransferee = null ;
					m_callTransferTarget = null ;

                    // Drop Consultative Call
                    try
                    {
                        call.setAutoDrop(true) ;
                        call.disconnect() ;
                        // Shell.getCallManager().showCall(call) ;
                    }
                    catch (PCallException ee)
                    {
                        Shell.getInstance().showUnhandledException(ee, false) ;
                    }
                    

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
            return "Dial the destination for this call now. You can announce the caller privately before completing the transfer." ;
        }


        /**
         * Gets the string based representation of this strategy.  This text is
         * typically prepended the the form name.
         */
        public String getFunction()
        {
            return "Transfer" ;
        }


        /**
         * Gets the string based representation of the dialing strategy action.
         * This text is typically displayed as the B3 button label
         */
        public String getAction()
        {
            return "Dial" ;
        }


        /**
         * Gets the hint text associatd with the dialing strategy action.  This
         * hint text is typically displayed when the B3 buttons is pressed and
         * held down.
         */
        public String getActionHint()
        {
            return "Dial|Dials this address so that you can transfer the call." ;
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
