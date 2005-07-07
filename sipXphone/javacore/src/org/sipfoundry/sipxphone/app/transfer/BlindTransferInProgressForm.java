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

package org.sipfoundry.sipxphone.app.transfer ;

import java.awt.* ;
import java.util.* ;
import java.text.* ;

import javax.telephony.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.telephony.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.service.* ;

import org.sipfoundry.sipxphone.sys.calllog.* ;

import org.sipfoundry.stapi.* ;
import org.sipfoundry.stapi.event.* ;

import javax.telephony.callcontrol.events.* ;


/**
 * The BlindTransferInProgressForm is responsible for display all of the transfer
 * state (for the Transfer Controller).  It smells ans acts much like the
 * WaitingForCallForm.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class BlindTransferInProgressForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final int ZERO = 0;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected icEventDispatcher      m_dispatcher = new icEventDispatcher() ;
    protected PContainer             m_container ;
    protected Component              m_calleeRenderer ;
    protected PLabel                 m_lblStatus ;
    protected String                 m_strStatus ;

    protected PCall                  m_callOriginal ;
    protected PCall                  m_callTransferMonitor ;
    protected boolean                m_bShownForm ;
    protected boolean                m_bFinalState ;

    protected icOriginalConnectionListener  m_originalConnectionListener ;
    protected icMonitorConnectionListener  m_monitorConnectionListener ;

    protected BlindTransferFailedForm m_formFailed ;
    
	protected int m_iCloseFormDelay = XpressaSettings.getHungUpFormCloseDelay();



    /**
     * Constructor requiring both the transferee (callOriginal) and the transfer
     * monitor call (call between the transferee and the transfer target).
     */
    public BlindTransferInProgressForm(Application application,
                                       PCall       callOriginal,
                                       PCall       callTransferMonitor)
    {
        super(application, "Transfer Status") ;
        m_bShownForm = false ;

        m_callOriginal = callOriginal ;
        m_originalConnectionListener = new icOriginalConnectionListener() ;
        m_callOriginal.addConnectionListener(m_originalConnectionListener) ;

        m_callTransferMonitor = callTransferMonitor ;
        m_monitorConnectionListener = new icMonitorConnectionListener() ;
        m_callTransferMonitor.addConnectionListener(m_monitorConnectionListener) ;

        setTitle("Transfer Status") ;

        initMenus() ;

        initializeComponents() ;

        setHelpText(getString("transfer_progress"), getString("transfer_progress_title")) ;

        addFormListener(new icFormListener()) ;
        setTimeDateTitle(true) ;
        m_bFinalState = false ;
    }


    /**
     * Updates the caller id portion of the screen
     */
    public void updateCallerID(PAddress address)
    {
        DefaultSIPAddressRenderer  renderer = new DefaultSIPAddressRenderer() ;
        Dimension                  dimSize = getSize() ;
        String                     strAddress = null ;

        if (address != null)
            strAddress = address.getAddress() ;

        Component comp = renderer.getComponent(this, strAddress, false) ;
        if (m_calleeRenderer != null) {
            m_container.remove(m_calleeRenderer) ;
        }

        m_calleeRenderer = comp ;
        m_calleeRenderer.setBounds(1, 54, dimSize.width-3, 27) ;
        m_container.add(m_calleeRenderer) ;

        // Force a repaint
        m_calleeRenderer.repaint() ;
    }


    /**
     * This method is call once the transfer attempt is made.
     */
    public void onTrying(PAddress address)
    {
        m_lblStatus.setText("Status: Transferring...") ;
        updateCallerID(address) ;
    }


    /**
     * As of implementation this is never being called.  We do not actually
     * know when the call starts ring (from a SIP level, see Dan Petrie for
     * details).
     */
    public void onRinging(PCall call, PAddress address)
    {
        m_lblStatus.setText("Status: Transferring...") ;
        updateCallerID(address) ;

        try {
            call.playTone(PtTerminalConnection.DTMF_TONE_RINGBACK, true, false) ;
        } catch (PSTAPIException e) {
            SysLog.log(e) ;
        }
    }


    /**
     * Invoked when the transfer has been completed.
     */
    public void onTransferComplete(PAddress address)
    {
        m_bFinalState = true ;

        if ((!m_bShownForm) || isInFocus())
        {
            // Update the GUI
            m_lblStatus.setText("Transfer: Complete") ;
            updateCallerID(address) ;

            // Show the close button before trying to disconnect (otherwise, it
            // take a while for the button to display)
            enableCloseButton(true) ;

            // Drop the Original call
            if (m_callOriginal != null)
            {
                TransferUtils.dropBlindTransferCall(m_callOriginal) ;
                m_callOriginal = null ;
            }

            // Drop the Monitor call
            if (m_callTransferMonitor != null)
            {
                TransferUtils.dropBlindTransferCall(m_callTransferMonitor) ;
                m_callTransferMonitor = null ;
            }
            
			if( m_iCloseFormDelay > 0 )
			{
				org.sipfoundry.sipxphone.service.Timer.getInstance().addTimer( m_iCloseFormDelay, m_dispatcher, m_dispatcher.ACTION_CLOSE)  ;
			}
        }
        else
        {
            onClose() ;
        }
    }


    /**
     * Invoked when the transfer has failed.  The Transfer control should
     * probably return to the orignal call, but we still give him the option
     * of leaving the call on hold.
     */
    public void onTransferFailed(PAddress address)
    {
        PCall callOriginal = m_callOriginal ;
        PCall callMonitor  = m_callTransferMonitor ;

        m_bFinalState = true ;

        //System.out.println("onTransferFailed") ;
        //System.out.println("  original:" + m_callOriginal.getCallID()) ;
        //System.out.println("   monitor:" + m_callTransferMonitor.getCallID()) ;

        // Restore original call to normal...
        if (callOriginal != null) {
            TransferUtils.unmarkBlindTransferCall(callOriginal) ;
            m_callOriginal = null ;
        }

        // Drop the Monitor call
        if (callMonitor != null)
        {
            TransferUtils.unmarkBlindTransferCall(callMonitor) ;
            TransferUtils.dropBlindTransferCall(callMonitor) ;
            m_callTransferMonitor = null ;
        }

        // Display failed status
        if (callOriginal != null)
        {
            m_formFailed = new BlindTransferFailedForm(ShellApp.getInstance().getCoreApp(), callOriginal, address) ;
            m_formFailed.showModeless() ;
            // Regardless of the state of any forms, make we close this form.
            onClose() ;
        }
        else
        {
            m_lblStatus.setText("Transfer: Failed") ;
        }
    }


    public void onClose()
    {
        closeForm() ;
    }
    

//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     *
     */
    private void initMenus()
    {
        PActionItem      items[] ;
        PBottomButtonBar menuControl = null ;

        // Initialize Menubar
        menuControl = getBottomButtonBar() ;
        menuControl.setItem(    PBottomButtonBar.B2,
                                new PLabel("Close"),
                                m_dispatcher.ACTION_CLOSE,
                                getString("hint/transfer/inprogress/close")) ;
        menuControl.addActionListener(m_dispatcher) ;

        // Initialize Left Menu
        items = new PActionItem[1] ;
        items[0] = new PActionItem(  new PLabel("Close"),
                                     getString("hint/transfer/inprogress/close"),
                                     m_dispatcher,
                                     m_dispatcher.ACTION_CLOSE) ;
        setLeftMenu(items) ;
    }


    /**
     *
     */
    private void enableCloseButton(boolean bEnable)
    {
        enableMenusByAction(bEnable, m_dispatcher.ACTION_CLOSE) ;
    }

    /**
     * Initialize the GUI
     */
    private void initializeComponents()
    {
        m_container = new PContainer() ;
        m_container.setLayout(null) ;

        m_lblStatus = new PLabel("") ;
        m_lblStatus.setText("Transfer: Trying...") ;
        m_lblStatus.setAlignment(PLabel.ALIGN_WEST) ;
        m_lblStatus.setBounds(1, 0, 150, 54) ;

        m_container.add(m_lblStatus) ;

        addToDisplayPanel(m_container, new Insets(1,1,1,1)) ;
    }


    public void doLayout()
    {
        Dimension   dimSize = getSize() ;

        super.doLayout() ;

        m_lblStatus.setBounds(1, 0, 150, 54) ;
        m_calleeRenderer.setBounds(1, 54, dimSize.width-3, 27) ;
    }




//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////
    /**
     * Dispatches various action commands to various handlers
     */
    public class icEventDispatcher implements PActionListener
    {
        public final String ACTION_CLOSE       = "action_close" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_CLOSE)) {
                onClose() ;
            }
			else if (event.getActionCommand().equals(org.sipfoundry.sipxphone.service.Timer.ACTION_TIMER_FIRED))
			{
				String strParam = (String) event.getObjectParam() ;
				if (strParam != null)
				{				
					if (strParam.equalsIgnoreCase(ACTION_CLOSE))
					{				
						onClose() ;
					}	
				}
			}            
        }
    }


    /**
     * Connection Listener look for state changes on the transferee
     */
    private class icOriginalConnectionListener implements PConnectionListener
    {
        public void callCreated(PConnectionEvent event) { }
        public void connectionTrying(PConnectionEvent event) { }
        public void connectionUnknown(PConnectionEvent event) { }
        public void connectionInboundAlerting(PConnectionEvent event) { }
        public void connectionOutboundAlerting(PConnectionEvent event) { }
        public void callHeld(PConnectionEvent event) { }
        public void callReleased(PConnectionEvent event) { }
        public void connectionConnected(PConnectionEvent event) { }


        /**
         * In transfer situations, the original connection should never fail.
         * If it does- something is very wrong.  We will clean up and drop all
         * of the calls.
         */
        public void connectionFailed(PConnectionEvent event)
        {

            // Only process if we haven't already hit a final state
            if (!m_bFinalState)
            {
                PCall call = event.getCall() ;
                PAddress address = event.getAddress() ;

                System.out.println("ORIGINAL: connectionFailed()") ;

                if (call != null) {
                    if (call.getConnectionState() <= PCall.CONN_STATE_DISCONNECTED) {
                        try
                        {
                            if (m_callOriginal != null)
                            {
                                TransferUtils.dropBlindTransferCall(m_callOriginal) ;
                                m_callOriginal = null ;
                            }
                            onTransferFailed(address) ;
                        }
                        catch (Exception e)
                        {
                            SysLog.log(e) ;
                        }
                    }
                }
            }
        }


        /**
         * In transfer situations, the original connection disconnecting is a
         * sign of success.  It is also possible that the caller hung up,
         * however, we currently cannot tell the difference without looking at
         * the monitor call/connection.
         */
        public void connectionDisconnected(PConnectionEvent event)
        {
            // Only process if we haven't already hit a final state
            if (!m_bFinalState)
            {
                PCall call = event.getCall() ;
                PAddress address = event.getAddress() ;

                System.out.println("ORIGINAL: connectionDisconnected()") ;

                if (call != null) {
                    int iState = call.getConnectionState() ;
                    if (iState <= PCall.CONN_STATE_DISCONNECTED) {
                        if (m_callOriginal != null)
                        {
                            TransferUtils.dropBlindTransferCall(m_callOriginal) ;
                            m_callOriginal = null ;
                            
							TransferUtils.dropBlindTransferCall(m_callTransferMonitor) ;
							m_callTransferMonitor = null ;                                                                                                           
                        }
                    }
                    else
                    {
                        System.out.println("**") ;
                        System.out.println("** Not dropping original call: " + call.getCallID()) ;
                        call.dumpConnections() ;
                        call.dumpConnectionsCache() ;
                        System.out.println("**") ;
                    }
                }
            }
        }


        public void callDestroyed(PConnectionEvent event)
        {
            System.out.println("ORIGINAL: callDestroyed()") ;

            if ((m_formFailed != null) && (m_formFailed.isShowing()))
            {
                m_formFailed.onClose() ;
            }
        }
    }


    /**
     * The connection listener on the monitor call informs us on the state of
     * the transfer.  These state changes drive the transfer process.
     */
    private class icMonitorConnectionListener implements PConnectionListener
    {
        public void callCreated(PConnectionEvent event) { }
        public void connectionTrying(PConnectionEvent event) { }
        public void connectionUnknown(PConnectionEvent event) { }
        public void connectionInboundAlerting(PConnectionEvent event) { }
        public void connectionOutboundAlerting(PConnectionEvent event) { }
        public void callReleased(PConnectionEvent event) { }


        /**
         * A connection event on the monitor connection suggestions/indicates
         * that the transfer was a success.  As a response, we will tear down
         * the the various calls and update the user interface.
         */
        public void connectionConnected(PConnectionEvent event)
        {
            // Only process if we haven't already hit a final state
            if (!m_bFinalState)
            {
                PCall call = event.getCall() ;
                PAddress address = event.getAddress() ;

                System.out.println("MONITOR: connectionConnected()") ;

                if (call != null) {
                    try
                    {
                        onTransferComplete(address) ;

                        if (m_callOriginal != null)
                        {
                            TransferUtils.dropBlindTransferCall(m_callOriginal) ;
                            m_callOriginal = null ;
                        }

                        if (m_callTransferMonitor != null)
                        {
                            TransferUtils.dropBlindTransferCall(m_callTransferMonitor) ;
                            m_callTransferMonitor = null ;
                        }
                    }
                    catch (Exception e)
                    {
                        System.out.println("Error trying to drop call: ") ;
                        SysLog.log(e) ;
                    }
                }
            }
        }


        /**
         * a failed event passed to the monitor indicates that the transfer
         * has failed.  This handler drop the monitor, but not the original
         * call.
         */
        public void connectionFailed(PConnectionEvent event)
        {
            // Only process if we haven't already hit a final state
            if (!m_bFinalState)
            {
                PCall call = event.getCall() ;
                PAddress address = event.getAddress() ;

                System.out.println("MONITOR: connectionFailed()") ;

                if (call != null)
                {
                    try
                    {
                        onTransferFailed(address) ;
                    }
                    catch (Exception e) {
                        System.out.println("Error trying to drop call: ") ;
                        SysLog.log(e) ;
                    }
                }
            }
        }


        public void callHeld(PConnectionEvent event)
        {
            if (m_callTransferMonitor != null)
                closeForm() ;
        }


        public void connectionDisconnected(PConnectionEvent event)
        {
            // Only process if we haven't already hit a final state
            if (!m_bFinalState)
            {
                PCall call = event.getCall() ;
                PAddress address = event.getAddress() ;

                System.out.println("") ;
                System.out.println("?????????????????????????????????") ;
                System.out.println("MONITOR: connectionDisconnected()") ;
                System.out.println("?????????????????????????????????") ;
                System.out.println("") ;

                if (call != null)
                {
                    try
                    {
                        onTransferFailed(address) ;
                    }
                    catch (Exception e)
                    {
                        System.out.println("Error trying to drop call: ") ;
                        SysLog.log(e) ;
                    }
                }
            }
            else
            {
                if (m_callTransferMonitor != null)
                {
                    TransferUtils.dropBlindTransferCall(m_callTransferMonitor) ;
                    m_callTransferMonitor = null ;
                }

            }
        }


        public void callDestroyed(PConnectionEvent event)
        {
            System.out.println("MONITOR: callDestroyed()") ;
        }
    }


    /**
     * Lists for various form changes and make sure the call is handed off
     * if needed
     */
    private class icFormListener extends PFormListenerAdapter
    {
        public void formOpening(PFormEvent event)
        {
            if (m_lblStatus != null)
                m_lblStatus.setText("Transfer: Trying...") ;
            enableCloseButton(true) ;
        }


        // If the form is closing and we still have an original call, then
        // the user must have aborted the transfer
        public void formClosing(PFormEvent event)
        {
        }


        public void focusGained(PFormEvent event)
        {
            m_bShownForm = true ;
        }



        // If we lose focus and the transfer monitor is null, we've finished!
        public void focusLost(PFormEvent event)
        {
            PForm formGainingFocus = event.getFormGainingFocus() ;

            if (    (formGainingFocus != null) &&
                    (!(formGainingFocus instanceof MessageBox)) &&
                    (!formGainingFocus.getFormName().equals("Task Manager")) &&
                    (!formGainingFocus.getFormName().equals(getTitle())) &&
                    (m_callTransferMonitor == null)) {
                closeForm() ;
            }
        }
    }
}
