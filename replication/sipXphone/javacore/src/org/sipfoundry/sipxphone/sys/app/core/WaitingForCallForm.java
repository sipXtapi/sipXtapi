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


package org.sipfoundry.sipxphone.sys.app.core ;

import java.awt.* ;
import java.util.* ;
import java.text.* ;

import javax.telephony.* ;

import org.sipfoundry.stapi.event.PConnectionEvent ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.telephony.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.service.* ;


import org.sipfoundry.sipxphone.sys.app.* ;

import org.sipfoundry.stapi.* ;

import javax.telephony.callcontrol.events.* ;

/**
 *
 */
public class WaitingForCallForm extends PApplicationForm
        implements CoreCallMonitor, DialingStrategyListener
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected icEventDispatcher      m_dispatcher = new icEventDispatcher() ;
    protected DTMFGenerator          m_DTMFGenerator = new DTMFGenerator() ;
    protected PContainer             m_container ;
    protected Component              m_calleeRenderer ;
    protected PLabel                 m_lblStatus ;
    protected boolean                m_bTransfering ;
    protected PCall                  m_call ;
    protected boolean                m_bPlayRingback ;

    /** PConnectionEvent when  the waited call failed. */
    protected PConnectionEvent m_failedConnectionEvent;

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     *
     */
    public WaitingForCallForm(Application application)
    {
        super(application, "Waiting For Call") ;
        m_call = null ;

        initializeMenus(false) ;

        initializeComponents() ;

        setHelpText(getString("waiting_for_call_to_go_through"), getString("waiting_for_call_to_go_through_title")) ;

        m_bTransfering = false ;

        // Allows the end-user to disable ringback.  This was a request from
        // Don Jackson at TellMe.
        String strRingbackSetting = PingerConfig.getInstance().getValue("PHONESET_RINGBACK") ;
        if ((strRingbackSetting != null) && strRingbackSetting.equals("DISABLE")) {
            m_bPlayRingback = false ;
        } else
            m_bPlayRingback = true ;

        setTimeDateTitle(true) ;
    }


    /**
     *
     */
    protected void initializeComponents()
    {
        m_container = new PContainer() ;
        m_container.setLayout(null) ;

        m_lblStatus = new PLabel("") ;
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

        if (m_calleeRenderer != null)
            m_calleeRenderer.setBounds(1, 54, dimSize.width-4, 27) ;
    }


    public void setTransfering(boolean bTransfering)
    {
        m_bTransfering = bTransfering ;
    }


    /**
     *
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
        m_calleeRenderer.setBounds(1, 54, dimSize.width-4, 27) ;
        m_container.add(m_calleeRenderer) ;

        // Force a repaint
        m_calleeRenderer.repaint() ;
    }


    /**
     *
     */
    public void onTrying(PCall call, PAddress address)
    {
        m_call = call ;

        if (m_bTransfering)
            m_lblStatus.setText("Transfer Status: Trying...") ;
        else
            m_lblStatus.setText("Status: Trying...") ;

        DialingStrategy strategy = Shell.getInstance().getDefaultDialingStrategy() ;
        strategy.addDialingStrategyListener(this) ;

        updateCallerID(address) ;
        initializeMenus(false) ;
    }


    /**
     *
     */
    public void onRinging(PCall call, PAddress address, int iCauseCode)
    {
        m_call = call ;

        if (m_bTransfering)
            m_lblStatus.setText("Transfer Status: Ringing...") ;
        else
            m_lblStatus.setText("Status: Ringing...") ;

        updateCallerID(address) ;

        // If the cause (code equals == PConnectionEvent.CAUSE_UNKNOWN) then
        // we have early media and should not play any ring back.
        if ((m_bPlayRingback) && (iCauseCode != PConnectionEvent.CAUSE_UNKNOWN))
        {
            try
            {
//              if (m_call.getConnectionStateUncached(address) == PCall.CONN_STATE_OUTBOUND_ALERTING)
//              {
                    // Make sure the audio device is enable
                    Shell.getMediaManager().setDefaultAudioDevice(false) ;

                    if (!call.isPlayingTone(PtTerminalConnection.DTMF_TONE_RINGBACK))
                        call.playTone(PtTerminalConnection.DTMF_TONE_RINGBACK, true, false) ;
//              }
            }
            catch (PSTAPIException e)
            {
                SysLog.log(e) ;
            }
        }
        else
        {
            // If we have early media, we should be listenting to button
            // presses and playing DTMF tones
            if (iCauseCode == PConnectionEvent.CAUSE_UNKNOWN)
            {
                addButtonListener(m_DTMFGenerator) ;
            }
        }

        DialingStrategy strategy = Shell.getInstance().getDefaultDialingStrategy() ;
        strategy.addDialingStrategyListener(this) ;


        initializeMenus(false) ;
    }

    /**
     * morphes the form to show  after a call is dropped.
     * An "info" button is shown which when pressed shows the causeCode,
     * sip response code and sip response text( if any ).
     * @param call The call that failed.
     * @param event PConnectionEvent when the call failed.
     * @see public void onFailed(PCall call, PAddress address, int cause)
     */
    public void onFailed(PCall call, PConnectionEvent event )

    {
        m_failedConnectionEvent = event;
        onFailed( call, event.getAddress(), event.getCause() );
        PActionItem actionInfo = new PActionItem( new PLabel("Info"),
                    getString("hint/coreapp/failedcall/info"),
                    m_dispatcher,
                    m_dispatcher.ACTION_FAILED_CALL_INFO) ;
        getBottomButtonBar().setItem(PBottomButtonBar.B1, actionInfo) ;
        PMenuComponent leftMenu = getLeftMenuComponent() ;
        leftMenu.addItem(actionInfo);
    }


    /**
     *
     */
    public void onFailed(PCall call, PAddress address, int iCauseCode)
    {
        m_call = call ;

        updateCallerID(address) ;
        if (m_bTransfering)
        {
            m_lblStatus.setText(getString("lblFailedCallTransfer")) ;
        }
        else
        {
            switch (iCauseCode)
            {
                case PConnectionEvent.CAUSE_BUSY:
                    m_lblStatus.setText(getString("lblFailedCallBusy")) ;
                    try
                    {
                        call.playTone(PtTerminalConnection.DTMF_TONE_BUSY) ;
                    }
                    catch (PSTAPIException e)
                    {
                        SysLog.log(e) ;
                    }
                    break ;
                case PConnectionEvent.CAUSE_CALL_CANCELLED:
                    m_lblStatus.setText(getString("lblFailedCallNoAnswer")) ;
                    try
                    {
                        call.playTone(PtTerminalConnection.DTMF_TONE_CALLFAILED) ;
                    }
                    catch (PSTAPIException e)
                    {
                        SysLog.log(e) ;
                    }
                    break ;
                default:
                    if (call.getStartTime() != null)
                    {
                        m_lblStatus.setText(getString("lblFailedCallNegotiate")) ;
                    }
                    else
                    {
                        m_lblStatus.setText(getString("lblFailedCallUnknown")) ;
                    }

                    try
                    {
                        call.playTone(PtTerminalConnection.DTMF_TONE_CALLFAILED) ;
                    }
                    catch (PSTAPIException e)
                    {
                        SysLog.log(e) ;
                    }
                    break ;
            }
        }

        DialingStrategy strategy = Shell.getInstance().getDefaultDialingStrategy() ;
        strategy.addDialingStrategyListener(this) ;

        initializeMenus(true) ;
    }


    /**
     * Get the call being monitored by this CoreCallMonitor implementation.
     */
    public PCall getMonitoredCall()
    {
        return m_call ;
    }



    /**
     * called when the form is being closed.
     */
    public void onFormClosing()
    {
        // Since this form is recycled, it is best to explicitly null out
        // these references so they can be properly garbage collected.
        if (m_calleeRenderer != null) {
            m_container.remove(m_calleeRenderer) ;
        }
        m_call = null ;

        DialingStrategy strategy = Shell.getInstance().getDefaultDialingStrategy() ;
        strategy.removeDialingStrategyListener(this) ;
        //remove the info button..don't let it hang around.
        if (getBottomButtonBar().getItem(PBottomButtonBar.B1) != null)
        {
            PActionItem b1Item = this.getBottomButtonBar().getItem(PBottomButtonBar.B1);
            if( b1Item.getActionCommand() == m_dispatcher.ACTION_FAILED_CALL_INFO )
                getBottomButtonBar().clearItem(PBottomButtonBar.B1);
        }

        removeButtonListener(m_DTMFGenerator) ;
    }



    /**
     *
     */
    protected void initializeMenus(boolean bDisconnected)
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;
        PMenuComponent   leftMenu = getLeftMenuComponent() ;
        PMenuComponent   rightMenu = getRightMenuComponent() ;
        PActionItem      actionHangup = null ;
        PActionItem      actionHangupAbort = null ;
        boolean bIsTransfer = false;

        // If we have a hangup handler, create a actionHangup ActionItem
        if (m_call != null)
        {
            ActionHandler handler = m_call.getHangupHandler() ;
            if (handler != null)
            {
                String strHangupHandlerLabel = handler.getLabel() ;

                //big hack-o-matic.  handler's text is passed here.
                //so the only way I know it's a transfer is to check the text.
                //But what if the text changes???
                if (strHangupHandlerLabel == "Transfer")
                    bIsTransfer = true;

                String strHangupHandlerHint = handler.getHint() ;

                if (strHangupHandlerHint == null)
                {
                    strHangupHandlerHint = strHangupHandlerHint ;
                }

                // Create Hangup handler
                actionHangup = new PActionItem(new PLabel(strHangupHandlerLabel),
                        strHangupHandlerLabel,
                        m_dispatcher,
                        m_dispatcher.ACTION_HANGUP) ;

                // Create Hangup abort handler
                if (bIsTransfer)
                    actionHangupAbort = new PActionItem(new PLabel("Cancel"),
                            getString("hint/core/system/cancelform"),
                                m_dispatcher,
                                m_dispatcher.ACTION_HANGUP_ABORT) ;
                else
                    actionHangupAbort = new PActionItem(new PLabel("Disconnect"),
                            getString("hint/coreapp/inprogress/disconnect"),
                                m_dispatcher,
                                m_dispatcher.ACTION_HANGUP_ABORT) ;

            }
        }

        //TextUtils.debug("action hangup is "+ actionHangup);
        //TextUtils.debug("bIsTransfer is " + bIsTransfer );
        //TextUtils.debug("m_bTransfering is "+ m_bTransfering );

        // Initialize Button Bars
        if (!bDisconnected)
        {

            if (actionHangup != null)
            {
                menuControl.setItem(PBottomButtonBar.B2, actionHangupAbort) ;
                if (bIsTransfer)
                {
                    menuControl.setItem(PBottomButtonBar.B1, actionHangup) ;
                }else
                {
                    menuControl.setItem(PBottomButtonBar.B3, actionHangup) ;
                }
            }
            else
            {
                PActionItem actionDisconnect = new PActionItem( new PLabel("Disconnect"),
                        getString("hint/coreapp/waiting/disconnect"),
                        m_dispatcher,
                        m_dispatcher.ACTION_DISCONNECT) ;
                menuControl.setItem(PBottomButtonBar.B2, actionDisconnect) ;
                menuControl.clearItem(PBottomButtonBar.B1) ;

                if (!m_bTransfering)
                {
                    menuControl.clearItem(PBottomButtonBar.B3) ;
                }

            }
        }
        else
        {
            // Menu Bar
            PActionItem actionClose = new PActionItem( new PLabel("Close"),
                    getString("hint/core/system/closeform"),
                    m_dispatcher,
                    m_dispatcher.ACTION_DISCONNECT) ;
            menuControl.setItem(PBottomButtonBar.B2, actionClose) ;
        }


        // Update Left Menu
        if (!bDisconnected)
        {
            if (actionHangup != null)
            {
                leftMenu.addItem(actionHangup) ;
                leftMenu.addItem(actionHangupAbort) ;
            }
            else
            {
                // Menu
                PActionItem items[] = new PActionItem[1] ;
                items[0] = new PActionItem( new PLabel("Disconnect"),
                                            getString("hint/coreapp/waiting/disconnect"),
                                            m_dispatcher,
                                            m_dispatcher.ACTION_DISCONNECT) ;
                setLeftMenu(items) ;
            }
        }
        else
        {
            // Menu
            PActionItem items[] = new PActionItem[1] ;
            items[0] = new PActionItem( new PLabel("Close"),
                                        getString("hint/core/system/closeform"),
                                        m_dispatcher,
                                        m_dispatcher.ACTION_DISCONNECT) ;
            setLeftMenu(items) ;
        }



        // Update right Menu
        rightMenu.removeAllItems();
        rightMenu.addItem(new PActionItem( new PLabel("About"),
                                    getString("hint/core/system/aboutform"),
                                    m_dispatcher,
                                    m_dispatcher.ACTION_ABOUT)) ;
    }


    /**
     * initialize our status to "" whenever the form is initially shown
     */
    public synchronized int showModal()
    {
        if (m_lblStatus != null)
            m_lblStatus.setText("") ;

        return super.showModal() ;
    }


    /**
     * initialize our status to "" whenever the form is initially shown
     */
    public synchronized boolean showModeless()
    {
        if (m_lblStatus != null)
            m_lblStatus.setText("") ;

        return super.showModeless() ;
    }




    /**
     * Initialize the menus that are displayed when a user hits the pingtel
     * key and selects the menu tab.
     */
    protected void initializeMenus()
    {
        PActionItem items[] ;

        // Initialize left menu

        /** leave blank */

        // Initialize right menu


        items = new PActionItem[1] ;

        items[0] = new PActionItem( new PLabel("Disconnect"),
                                    getString("hint/coreapp/waiting/disconnect"),
                                    m_dispatcher,
                                    m_dispatcher.ACTION_DISCONNECT) ;
        setRightMenu(items) ;


    }


    /**
     *
     */
    public void onDisconnect()
    {
        PCall call = m_call ;

        closeForm() ;

        if (call != null)
        {
            // Turn off the audio device
            try
            {
                Shell.getMediaManager().setAudioDevice(PAudioDevice.ID_NONE) ;
            }
            catch (PMediaException e)
            {
                SysLog.log(e) ;
            }

            // Drop the call
            try
            {
                call.disconnect() ;
            }
            catch (PCallException e)
            {
                SysLog.log(e) ;
            }
        }


        if (m_bTransfering)
        {
            ((CoreApp) getApplication()).doHold() ;
        }
    }


    public void onActionHangup()
    {
        PCall call = m_call ;

        // If we have a hangup handler, create a actionHangup ActionItem
        if (call != null)
        {
            ActionHandler handler = m_call.getHangupHandler() ;
            if (handler != null)
            {
                if (handler.performAction("ACTION_HANGUP", call, null))
                    closeForm() ;
            }
            else
            {
                closeForm() ;
            }
        }
        else
        {
            closeForm() ;
        }
    }


     /**
     * shows up a SimpleTextForm box with causeCode, sip errorcode and sip errortext.
     */
    public void onActionShowFailedCallInfo(PConnectionEvent event)
    {
        // Turn off the audio device; stop tones
        PCall call = m_call ;
        if (call != null)
        {
            try
            {
                Shell.getMediaManager().setAudioDevice(PAudioDevice.ID_NONE) ;
                call.stopTone() ;
                call.stopSound() ;
            }
            catch (Exception e)
            {
                SysLog.log(e) ;
            }
        }

        // Display failed call info
        ((CoreApp) getApplication()).doShowFailedCallInfo(event) ;
    }


    public void onActionHangupAborted()
    {
        PCall call = m_call ;
        // If we have a hangup handler, create a actionHangup ActionItem
        if (call != null)
        {
            ActionHandler handler = m_call.getHangupHandler() ;
            if (handler != null)
            {
                if (handler.performAction("ACTION_ABORTED", call, null))
                    closeForm() ;
            }
            else
            {
                closeForm() ;
            }
        }
        else
        {
            closeForm() ;
        }
    }



    public void dialingInitiated(PCall call, PAddress address)
    {
        // Do nothing
    }

    public void dialingAborted(PCall call)
    {
        closeForm() ;
    }



    /**
     *
     */
    public class icEventDispatcher implements PActionListener
    {
        public final String ACTION_DISCONNECT   = "action_disconnect" ;
        public final String ACTION_ABOUT        = "action_about" ;
        public final String ACTION_HANGUP       = "action_hangup" ;
        public final String ACTION_HANGUP_ABORT = "action_hangup_abort" ;
        public final String ACTION_FAILED_CALL_INFO = "action_failed_call_info";

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_ABOUT))
            {
                SystemAboutBox box = new SystemAboutBox(getApplication());
                box.onAbout();
            }
            else if (event.getActionCommand().equals(ACTION_DISCONNECT))
            {
                onDisconnect() ;
            }
            else if (event.getActionCommand().equals(ACTION_HANGUP))
            {
                onActionHangup() ;
            }
            else if (event.getActionCommand().equals(ACTION_HANGUP_ABORT))
            {
                onActionHangupAborted() ;
            }
            else if (event.getActionCommand().equals(ACTION_FAILED_CALL_INFO))
            {
                onActionShowFailedCallInfo(m_failedConnectionEvent) ;
            }
        }
    }
}
