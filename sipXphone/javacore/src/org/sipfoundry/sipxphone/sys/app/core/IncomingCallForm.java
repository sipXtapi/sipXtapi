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

import java.io.* ;
import java.awt.* ;
import java.lang.reflect.*;
//import java.util.* ;
import java.text.* ;

import javax.telephony.* ;
import javax.telephony.phone.PhoneLamp ;
import javax.telephony.callcontrol.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.stapi.event.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.hook.* ;
import org.sipfoundry.sipxphone.app.* ;
import org.sipfoundry.sipxphone.sys.startup.*;
/**
 * Form displayed when an incoming call is dectected.  This form is displayed
 * for both new calls and caller waiting calls.
 */
public class IncomingCallForm extends PApplicationForm
    implements CoreCallMonitor
{

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected icEventDispatcher      m_dispatcher = new icEventDispatcher() ;
    protected PContainer             m_container ;
    protected Component              m_callerRenderer ;
    protected Component              m_calledRenderer ;
    protected PCall                  m_incomingCall ;
    protected boolean                m_bPlayingAudio = false ;
    protected CountingSemaphore      m_semaProcessingAlert ;
    protected Application            m_application;
    protected PAddress               m_dialBackAddress;
    protected MissedCallFeatureIndicator m_missedCalledFeatureIndicator ;
    protected boolean                m_callMissed = false;

    /** PConnectionEvent when  the incoming call failed */
    protected PConnectionEvent m_failedConnectionEvent;

     /**
     * the delay in closing a "hung up", "Call Disconnected" or
     * "Caller Redirected" form
     */
    protected static int m_iCloseFormDelay =
                XpressaSettings.getHungUpFormCloseDelay();

    /**
     *
     */
    public IncomingCallForm(Application application)
    {
        super(application, "Incoming Call") ;
        m_application = application;
        initializeMenubar() ;

        initializeComponents() ;
        initializeMenus(true) ;

        m_semaProcessingAlert = new CountingSemaphore(0, true) ;

    }


    protected void initializeComponents()
    {
        Dimension   dimSize = getSize() ;

        m_container = new PContainer() ;
        m_container.setLayout(null) ;

        addToDisplayPanel(m_container, new Insets(1,1,1,2)) ;
    }


    public void doLayout()
    {
        super.doLayout() ;

        Dimension dimSize = m_container.getSize() ;
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

        if (m_callerRenderer != null)
        {
            m_container.remove(m_callerRenderer) ;
        }

        m_callerRenderer = new ComponentBorder("From", comp) ;
        m_callerRenderer.setBounds(1, 0, dimSize.width-3, 50) ;
        m_container.add(m_callerRenderer) ;

        // Force a repaint
        m_callerRenderer.repaint() ;
    }


    /**
     * Updated the called (or local) address
     */
    public void updateCalledID(PAddress address)
    {
        DefaultSIPAddressRenderer  renderer = new DefaultSIPAddressRenderer() ;
        Dimension                  dimSize = getSize() ;
        String                     strAddress = null ;

        if (address != null)
            strAddress = address.getAddress() ;

        Component comp = renderer.getComponent(this, strAddress, false) ;

        if (m_calledRenderer != null)
        {
            m_container.remove(m_calledRenderer) ;
        }

        m_calledRenderer = new ComponentBorder("To", comp) ;
        m_calledRenderer.setBounds(1, 54, dimSize.width-3, 50) ;
        m_container.add(m_calledRenderer) ;

        // Force a repaint
        m_calledRenderer.repaint() ;
    }


    /**
     * Get the call being monitored by this CoreCallMonitor implementation.
     */
    public PCall getMonitoredCall()
    {
        return m_incomingCall ;
    }


    public void onIncomingCall(PCall call, PAddress address, int cause)
    {
        //remove the timer that closes the form.
        Timer.getInstance().removeTimers(m_dispatcher);

        String strDefaultRingFile = PingerInfo.getInstance().getDefaultRingFileName() ;
        PCall  callInFocus = Shell.getCallManager().getInFocusCall() ;
        m_incomingCall = call ;

        setTitle("Receiving Call...") ;

        updateCallerID(address) ;

        PAddress addressCalled = call.getCalledAddress() ;
        if (addressCalled != null)
        {
            updateCalledID(addressCalled) ;
        }

        // Only Ring if settings dictate it
        String strRingerMode = PingerConfig.getInstance().getValue(PingerConfig.PHONESET_RINGER) ;
        if ((strRingerMode == null) || (strRingerMode.equalsIgnoreCase("BOTH") || strRingerMode.equalsIgnoreCase("AUDIBLE"))) {

            // If no call is in focus,or if the call in focus is still ringing,
            // continue ringing for this new call, otherwise play a call waiting tone
            if ((callInFocus == m_incomingCall) || (callInFocus == null))
            {
                try {
                    m_incomingCall.releaseFromHold() ;
                } catch (PSTAPIException e) {
                    // ::TODO:: Handle this failure condition
                    SysLog.log(e) ;
                }

                AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;
                asc.enableRinger(true) ;
                HookManager manager = Shell.getHookManager() ;
                manager.executeHook(HookManager.HOOK_RINGER, new RingerHookData(address, strDefaultRingFile)) ;
                m_bPlayingAudio = true ;
            }
            else
            {
                //if the callInFocus is already connected, ie if not ringing,
                //play the call waiting tone, otherwise continue ringing
                if(( callInFocus != null ) &&
                    (callInFocus.getConnectionState() != PCall.CONN_STATE_INBOUND_ALERTING)){
                    try
                    {
                        int iPlayLength = Settings.getInt("CALL_WAITING_TONE_LENGTH", 200) ;
                        callInFocus.playTone(PtTerminalConnection.DTMF_TONE_CALLWAITING, iPlayLength, true, false) ;
                    }
                    catch (PSTAPIException e)
                    {
                        // ::TODO:: Handle this failure condition
                        SysLog.log(e) ;
                    }
                }else{
                    //keep ringing.
                }

            }

        }

        if ((strRingerMode == null) || (!strRingerMode.equalsIgnoreCase("AUDIBLE")))
        {
            Shell.getInstance().setMessageIndicator(PhoneLamp.LAMPMODE_FLUTTER, "Incoming Call") ;
            new icVisualCallIndicationResetter(call) ;

            //only if we allow visual notification & we are on ix
            //we should bring the app into focus.
            if (PingerApp.isTestbedWorld() == true)
            {
                String strBlockBringToFocus = PingerConfig.getInstance().getValue("PHONESET_BLOCK_FOCUS_CHANGE_ON_CALL") ;
                if (strBlockBringToFocus != null && strBlockBringToFocus == "YES")
                    ; //dont bring to focus
                else
                {
                    try
                    {
                        Class c = Class.forName("org.sipfoundry.sipxphone.testbed.TestbedFrame");
                        Method method = c.getMethod("bringToTop", null) ;
                        if (method != null) {
                            method.invoke(c, null) ;
                            m_callerRenderer.repaint() ;
                            repaint();
                        }
                    }
                    catch(Exception e)
                    {
                        System.out.println("IncomingCallForm ... Could not find class: org.sipfoundry.sipxphone.testbed.TestbedFrame");
                        e.printStackTrace();
                    }
                }
            }
        }


        setHelpText(getString("receiving_call"), getString("receiving_call_title")) ;
        initializeMenubar() ;
        initializeMenus(true) ;
        updateShowIncomingCallsButton();

        if (m_semaProcessingAlert.getCount() <= 0)
            m_semaProcessingAlert.up() ;
    }


    /**
     * morphes the form to show  after a call is dropped.
     * An "info" button is shown which when pressed shows the causeCode,
     * sip response code and sip response text( if any ).
     * @param call The call that failed.
     * @param event PConnectionEvent when the call failed.
     * @see public void onCallDropped(PCall call, PAddress address, int cause)
     */
    public void onCallDropped(PCall call, PConnectionEvent event )

    {
        m_failedConnectionEvent = event;
        onCallDropped( call, event.getAddress(), event.getCause() );
        PActionItem actionInfo = new PActionItem( new PLabel("Info"),
                    getString("hint/coreapp/failedcall/info"),
                    m_dispatcher,
                    m_dispatcher.ACTION_FAILED_CALL_INFO) ;
        getBottomButtonBar().setItem(PBottomButtonBar.B1, actionInfo) ;
        PMenuComponent leftMenu = getLeftMenuComponent() ;
        leftMenu.addItem(actionInfo);

    }


    public void onCallDropped(PCall call, PAddress address, int cause)
    {
        if( m_iCloseFormDelay > 0 )
        {
            Timer.getInstance().addTimer( m_iCloseFormDelay, m_dispatcher, null )  ;
        }

        if( m_missedCalledFeatureIndicator == null )
            m_missedCalledFeatureIndicator =
                  Shell.getCallManager().getMissedCallFeatureIndicator();

         m_dialBackAddress = null;
        // Display Status message based on cause code
        switch (cause) {
            case PConnectionEvent.CAUSE_REDIRECTED:
            case PConnectionEvent.CAUSE_BUSY:
                setTitle("Caller Redirected") ;
                String strAddressRedirected =
                   (String)(call.getCallData(PCallManager.REDIRECTED_ADDRESS));
                if (strAddressRedirected != null)
                {
                    try{
                        updateCalledID(PAddressFactory.getInstance().createAddress(strAddressRedirected)) ;
                    }catch( PCallAddressException e ){
                        SysLog.log(e);
                    }
                }

                break ;
            case PConnectionEvent.CAUSE_TRANSFER:
                setTitle("Caller transferred") ;
                break ;
            default:
                setTitle("Caller Hung up") ;
                m_callMissed = true;
                m_missedCalledFeatureIndicator.callMissed(call);
                break ;
        }
        getBottomButtonBar().clearItem(PBottomButtonBar.B3) ;

        if( m_callMissed ){
            if( m_iCloseFormDelay > 0 )
            {
                PActionItem actionDialBack = new PActionItem( new PLabel("Dial Back"),
                getString("hint/coreapp/incomingcall/dialback"),
                m_dispatcher,
                m_dispatcher.ACTION_DIAL_BACK) ;

                getBottomButtonBar().setItem(PBottomButtonBar.B3, actionDialBack) ;
                m_dialBackAddress = address;

            }else
            {
                //close the form.
                onCancel();
            }
        }

        PActionItem actionCancel = new PActionItem( new PLabel("Close"),
                getString("hint/core/system/closeform"),
                m_dispatcher,
                m_dispatcher.ACTION_CANCEL) ;

        getBottomButtonBar().setItem(PBottomButtonBar.B2, actionCancel) ;
        updateShowIncomingCallsButton();
        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;
        asc.enableRinger(false) ;
        try
        {
            call.stopTone() ;
            call.stopSound() ;
        } catch (PCallStateException e) {
            SysLog.log(e) ;
        }

        m_bPlayingAudio = false ;
        m_incomingCall = null ;

        initializeMenus(false) ;
    }


    /**
     *
     */
    protected void initializeMenubar()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        PActionItem actionIgnore = new PActionItem( new PLabel(getString("lblIncomingCallIgnore")),
                getString("hint/coreapp/incomingcall/ignore"),
                m_dispatcher,
                m_dispatcher.ACTION_IGNORE) ;
        menuControl.setItem(PBottomButtonBar.B2, actionIgnore) ;


        PActionItem actionAnswer = new PActionItem(new PLabel(getString("lblIncomingCallAnswer")),
                getString("hint/coreapp/incomingcall/answer"),
                m_dispatcher,
                m_dispatcher.ACTION_ANSWER) ;
        menuControl.setItem(PBottomButtonBar.B3, actionAnswer) ;

        PActionItem actionShowIncomingCalls = new PActionItem(new PLabel("Incoming Calls"),
                getString("hint/coreapp/incomingcall/show_incoming_calls"),
                m_dispatcher,
                m_dispatcher.ACTION_SHOW_INCOMING_CALLS) ;
        menuControl.setItem(PBottomButtonBar.B1, actionShowIncomingCalls) ;
        enableMenusByAction(false, m_dispatcher.ACTION_SHOW_INCOMING_CALLS);

    }





    /**
     * Initialize the menus that are displayed when a user hits the pingtel
     * key and selects the menu tab.
     */
    protected void initializeMenus(boolean bActive)
    {
        PActionItem items[] ;
        PMenuComponent rightMenu = getRightMenuComponent() ;
        PMenuComponent leftMenu  = getLeftMenuComponent() ;

        leftMenu.removeAllItems();
        rightMenu.removeAllItems();

        if (bActive) {
            items = new PActionItem[3] ;
            items[0] = new PActionItem( new PLabel(getString("lblIncomingCallAnswer")),
                                        getString("hint/coreapp/incomingcall/answer"),
                                        m_dispatcher,
                                        m_dispatcher.ACTION_ANSWER) ;
            items[1] = new PActionItem( new PLabel(getString("lblIncomingCallIgnore")),
                                        getString("hint/coreapp/incomingcall/ignore"),
                                        m_dispatcher,
                                        m_dispatcher.ACTION_IGNORE) ;

            items[2] = new PActionItem(new PLabel("Incoming Calls"),
                getString("hint/coreapp/incomingcall/show_incoming_calls"),
                m_dispatcher,
                m_dispatcher.ACTION_SHOW_INCOMING_CALLS) ;

            setLeftMenu(items) ;
        } else {
            items = new PActionItem[2] ;
            items[0] = new PActionItem( new PLabel("Close"),
                                        getString("hint/core/system/closeform"),
                                        m_dispatcher,
                                        m_dispatcher.ACTION_CANCEL) ;
            if (m_callMissed)
                items[1] = new PActionItem( new PLabel("Dial Back"),
                                        getString("hint/coreapp/incomingcall/dialback"),
                                        m_dispatcher,
                                        m_dispatcher.ACTION_DIAL_BACK) ;
            setLeftMenu(items) ;


        }

        rightMenu.addItem(new PActionItem( new PLabel("About"),
                                    getString("hint/core/system/aboutform"),
                                    m_dispatcher,
                                    m_dispatcher.ACTION_ABOUT)) ;

        enableMenusByAction(false, m_dispatcher.ACTION_SHOW_INCOMING_CALLS);

    }


    /**
     * cancel the incoming call (close the form)
     */
    public void onCancel()
    {
        closeForm() ;
    }


     public void onFocusGained(PForm formLosingFocus){
           updateShowIncomingCallsButton();
     }

    /**
     * answer the incoming call
     */
    public void onAnswer()
    {
        if (m_incomingCall != null) {
            PCall callInFocus = Shell.getCallManager().getInFocusCall() ;
            if (callInFocus != m_incomingCall) {
                if (callInFocus != null) {
                    if( callInFocus.getConnectionState() >= PCall.CONN_STATE_CONNECTED){
                        try {
                            callInFocus.placeOnHold() ;
                        } catch (PSTAPIException e) {
                            // ::TODO:: Handle this failure condition
                            SysLog.log(e) ;
                        }
                    }

                }
            }

            try {
                try {
                    m_incomingCall.stopSound() ;
                    m_bPlayingAudio = false ;
                } catch (Exception e) { }
                m_incomingCall.answer() ;
                m_incomingCall.releaseFromHold() ;
                Shell.getMediaManager().setDefaultAudioDevice() ;
            } catch (PSTAPIException e) {
                // ::TODO:: Handle this failure condition
                SysLog.log(e) ;
            }
        } else {
           closeForm() ;
        }
    }


    /**
     * Do not disturb me any more
     */
    public void onIgnore()
    {
        try {
            //if mute is on the shut it off
            PMediaManager   mediaManager = Shell.getMediaManager() ;
            PAudioDevice audioDevice = mediaManager.getAudioDevice() ;
            if (audioDevice != null)
            {
                try
                {
                    if (audioDevice.isMuted())
                        audioDevice.unmute() ;
                }
                catch (PMediaException e)
                {
                    SysLog.log(e) ;
                }
            }

            AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;
            m_incomingCall.stopSound() ;
            m_incomingCall.stopTone() ;
            m_bPlayingAudio = false ;
            asc.enableRinger(false) ;
            Shell.getCallManager().zombieCall(m_incomingCall) ;
            Shell.getCallManager().callNotRinging(m_incomingCall);
            m_incomingCall.placeOnHold() ;
            Shell.getInstance().setMessageIndicator(PhoneLamp.LAMPMODE_OFF, "Incoming Call") ;


        } catch (PSTAPIException e) {
            // ::TODO:: Handle this failure condition
            SysLog.log(e) ;
        }

        closeForm() ;
    }

    /**
     * When the user presses "Dial Back" button,
     * the phone dials the caller whose call was missed.
     */
    public void onDialBack(){

        //see if there is a hardcoded address to dial back.
        //if that is null, than use the current missed call's address.
        String strDialBackAddress =
            PingerConfig.getInstance().getValue(PingerConfig.PHONESET_DIAL_BACK_ADDRESS) ;

        if( strDialBackAddress != null )
            m_dialBackAddress = new PAddress(strDialBackAddress);
        if (m_dialBackAddress != null) {
            Shell.getInstance().getDialingStrategy().dial(m_dialBackAddress) ;
        } else {
            //shouldn't get here.
            getApplication().showMessageBox
                (MessageBox.TYPE_ERROR, "Cannot dial back.", "Cannot dial back.") ;
            Shell.getInstance().getDialingStrategy().abort() ;
        }
    }

    /**
     * when "incoming calls" buton is pressed,
     * it shows a form of incoming calls where an
     * user can select a list of incoming ringing cals.
     */
    public void onShowIncomingCalls(){
       ((CoreApp) getApplication()).doAnswerRingingCall() ;
    }

    /**
     * shows up a SimpleTextForm box with causeCode, sip errorcode and sip errortext.
     */
    public void onShowFailedCallInfo(PConnectionEvent event){
      ((CoreApp) getApplication()).doShowFailedCallInfo(event) ;
    }




    /**
     * called when the form is being closed.
     */
    public void onFormClosing()
    {
        System.out.println("**********************************") ;
        System.out.println("IncomingCallForm : onFormClosing") ;
        System.out.println("**********************************") ;

        // Since this form is recycled, it is best to explicitly null out
        // these references so they can be properly garbage collected.
        if (m_callerRenderer != null)
        {
            m_container.remove(m_callerRenderer) ;
        }
        if (m_calledRenderer != null)
        {
            m_container.remove(m_calledRenderer) ;
        }

        // Wait for onIncomingCall to finish processing.
        if  (m_semaProcessingAlert.getCount() <= 0)
        {
            new icWaiterForProcessingChange(m_incomingCall, m_semaProcessingAlert).start() ;
        }
        else {
            // Additionally, stop and sounds/tones if the ringer is enabled.
            if ((m_incomingCall != null) && m_bPlayingAudio)
            {
                PMediaManager manager = (PMediaManager) Shell.getMediaManager() ;
                PAudioDevice audioDevice = manager.getAudioDevice() ;
                if (audioDevice != null)
                {
                    if (audioDevice.getID() == PAudioDevice.ID_RINGER)
                    {
                        System.out.println("**********************************") ;
                        System.out.println("STOPPING TONES!") ;
                        System.out.println("**********************************") ;

                        try {
                            m_incomingCall.stopSound() ;
                            m_incomingCall.stopTone() ;
                        } catch (Exception e) { }
                        m_bPlayingAudio = false ;

                        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;
                        asc.enableRinger(false) ;
                    }
                }
            }
        }
        //remove the info button..don't let it hang around.
        if(  this.getBottomButtonBar().getItem(PBottomButtonBar.B1)  !=  null){
            PActionItem b1Item = this.getBottomButtonBar().getItem(PBottomButtonBar.B1);
            if( b1Item.getActionCommand() == m_dispatcher.ACTION_FAILED_CALL_INFO )
                this.getBottomButtonBar().clearItem(PBottomButtonBar.B1);
        }

        m_incomingCall = null ;
    }

    /**
     * decided whether to show "incoming calls" button
     * depending on the number of incoming ringing calls
     * and shows/hides it.
     * "Incoming calls" button is hidden if the number
     * of incoming calls is greater than one.
     */
    public void updateShowIncomingCallsButton(){
         PCall incomingCalls[] =
            Shell.getCallManager().getCallsByState(PCall.CONN_STATE_INBOUND_ALERTING) ;
         int validCallsCount = 0;
         PCallManager callManager = Shell.getCallManager();
         for( int i = 0; i<incomingCalls.length; i++){
            if (!callManager.isZombieCall(incomingCalls[i])
                && !callManager.isHiddenCall(incomingCalls[i]))
                validCallsCount++;

         }
         if( validCallsCount > 1 )
            enableMenusByAction(true, m_dispatcher.ACTION_SHOW_INCOMING_CALLS);
         else
            enableMenusByAction(false, m_dispatcher.ACTION_SHOW_INCOMING_CALLS);
    }


     /**
     * initialize our status to "" whenever the form is initially shown
     */
    public synchronized int showModal()
    {
//        if (m_lblStatus != null)
//            m_lblStatus.setText("") ;

        return super.showModal() ;
    }


    /**
     * initialize our status to "" whenever the form is initially shown
     */
    public synchronized boolean showModeless()
    {
//        if (m_lblStatus != null)
//            m_lblStatus.setText("") ;

        return super.showModeless() ;
    }





    /**
     * Connection listener that looks for callReleased and/or callDestroyed
     * messages.  If found, those calls are moved from the held call list
     */
    private class icConnectionListener extends PConnectionListenerAdapter
    {
        public void callCreated(PConnectionEvent event) {
        }

        public void callDestroyed(PConnectionEvent event) {
                updateShowIncomingCallsButton();
        }
        public void callHeld(PConnectionEvent event) {
                updateShowIncomingCallsButton();
        }
        public void callReleased(PConnectionEvent event) {
                updateShowIncomingCallsButton();
        }

        public void connectionTrying(PConnectionEvent event) {
            updateShowIncomingCallsButton();
        }

        public void connectionOutboundAlerting(PConnectionEvent event) {
            updateShowIncomingCallsButton();
        }

        public void connectionInboundAlerting(PConnectionEvent event) {
        }

        public void connectionConnected(PConnectionEvent event) {
        }

        public void connectionFailed(PConnectionEvent event) {
        }

        public void connectionUnknown(PConnectionEvent event) {
        }

        public void connectionDisconnected(PConnectionEvent event) {
        }

    }




    private class icWaiterForProcessingChange extends Thread
    {
        PCall m_call  ;
        CountingSemaphore m_semaWait ;

        public icWaiterForProcessingChange(PCall call, CountingSemaphore semaWait)
        {
            m_call = call ;
            m_semaWait = semaWait ;

        }

        public void run()
        {
            m_semaWait.down() ;

            // Additionally, stop and sounds/tones if the ringer is enabled.
            if ((m_call != null) && m_bPlayingAudio)
            {
                PMediaManager manager = (PMediaManager) Shell.getMediaManager() ;
                PAudioDevice audioDevice = manager.getAudioDevice() ;
                if (audioDevice != null)
                {
                    if (audioDevice.getID() == PAudioDevice.ID_RINGER)
                    {
                        try {
                            m_call.stopSound() ;
                            m_call.stopTone() ;
                        } catch (Exception e) { }
                        m_bPlayingAudio = false ;

                        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;
                        asc.enableRinger(false) ;
                    }
                }
            }

        }
    }


    private class icEventDispatcher implements PActionListener
    {
        public final String ACTION_IGNORE = "action_ignore" ;
        public final String ACTION_ANSWER = "action_answer" ;
        public final String ACTION_CANCEL = "action_cancel" ;
        public final String ACTION_ABOUT  = "action_about" ;
        public final String ACTION_SHOW_INCOMING_CALLS  = "action_show_incoming_calls" ;
        public final String ACTION_FAILED_CALL_INFO = "action_failed_call_info";
        public final String ACTION_DIAL_BACK = "action_dial_back";

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_ABOUT)) {
                SystemAboutBox box = new SystemAboutBox(getApplication());
                box.onAbout();
            }
            else if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            }
            else if (event.getActionCommand().equals(ACTION_ANSWER)) {
                onAnswer() ;
            }
            else if (event.getActionCommand().equals(ACTION_IGNORE)) {
                onIgnore() ;
            }
            else if (event.getActionCommand().equals(ACTION_SHOW_INCOMING_CALLS)) {
                onShowIncomingCalls() ;
            }
            else if (event.getActionCommand().equals(ACTION_FAILED_CALL_INFO)) {
                onShowFailedCallInfo(m_failedConnectionEvent) ;
            }
            else if (event.getActionCommand().equals(Timer.ACTION_TIMER_FIRED))
            {
                onCancel() ;
            }
            else if (event.getActionCommand().equals(ACTION_DIAL_BACK))
            {
                onDialBack() ;
            }

        }
    }



    private class icVisualCallIndicationResetter extends PConnectionListenerAdapter
    {
        protected PCall m_call ;

        public icVisualCallIndicationResetter(PCall call)
        {
            m_call = call ;
            m_call.addConnectionListener(this) ;
        }

        public void callDestroyed(PConnectionEvent event)
        {
            clearAndRemove() ;
        }

        public void callHeld(PConnectionEvent event)
        {
            clearAndRemove() ;
        }

        public void connectionConnected(PConnectionEvent event)
        {
            clearAndRemove() ;
        }


        public void connectionFailed(PConnectionEvent event)
        {
            clearAndRemove() ;
        }


        public void connectionDisconnected(PConnectionEvent event)
        {
            clearAndRemove() ;
        }


        protected void clearAndRemove()
        {
            m_call.removeConnectionListener(this) ;
            Shell.getInstance().setMessageIndicator(PhoneLamp.LAMPMODE_OFF, "Incoming Call") ;
        }
    }
}
