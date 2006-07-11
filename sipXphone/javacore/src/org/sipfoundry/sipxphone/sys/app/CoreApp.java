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

package org.sipfoundry.sipxphone.sys.app ;

import javax.telephony.* ;
import javax.telephony.phone.* ;
import javax.telephony.callcontrol.* ;

import java.util.* ;
import java.io.File;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.phone.* ;
import org.sipfoundry.telephony.phone.event.* ;
import org.sipfoundry.telephony.callcontrol.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.stapi.event.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.app.* ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.* ;
import org.sipfoundry.sipxphone.sys.app.core.* ;
import org.sipfoundry.sipxphone.sys.app.shell.* ;
import org.sipfoundry.sipxphone.hook.* ;
import org.sipfoundry.sipxphone.service.Timer ;
import org.sipfoundry.sipxphone.service.Logger ;

import org.sipfoundry.sipxphone.awt.*;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.app.adjustvolume.* ;

import org.sipfoundry.sip.SipLineManager ;
import org.sipfoundry.sip.SipLine ;


/**
 * Core application that manages/monitors state changes.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class CoreApp extends Application implements PButtonListener
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** what is the current state of the phone? */
    private int m_iLastState ;

    /** reference to the 'idle' form */
    private IdleForm            m_formIdle ;
    /** reference to the 'waiting for call to go through' form */
    private WaitingForCallForm  m_formWaitingForCall ;
    /** reference to the 'call in progress' form */
    private CallInProgressForm  m_formCallInProgress ;
    /** reference to the 'incoming call' form */
    private IncomingCallForm    m_formIncomingCall ;
    /** reference to the 'Dialing' form */
    private DialerForm          m_formDialer ;

    /** listening for hook switch notifications and what not. */
    private icTerminalListener  m_terminalListener ;
    private PhoneState          m_phoneState ;
    private FormManager         m_formManager ;

    private Vector              m_vIgnoreCalls ;

    private Vector              m_vZombieCalls ;

    private icCommandDispatcher m_coreListener = new icCommandDispatcher();

    /** Object used to synchronized form startup.  We need to make sure that
        we don't initialize the idle form until then CoreApp application is
        cooked (running).  */
    private CountingSemaphore  m_semStartUp = new CountingSemaphore(0, false) ;


    /** should this phone accept calls?  This is used as party of setup
        and also should probably be changed when performing operations
        such as upgrades, etc. */
    private boolean             m_bAcceptCalls ;

    /** Is this phone currently uploading a file */
    private boolean             m_bUploadingFile ;

    private GenericEventQueue   m_eventqueue ;

    private boolean             m_bStateDebug = false ;
    public static boolean DEBUG_EVENTS = false ;
    public static boolean DEBUG_METHODS = false ;

    private ApplicationIconCache m_iconCache ;

    private LineRegistrationFeatureIndicator m_fiLineRegistration ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public CoreApp()
    {
        m_vIgnoreCalls = new Vector() ;
        m_vZombieCalls = new Vector() ;
        m_bAcceptCalls = false ;
        m_bUploadingFile = false;
        m_eventqueue   = new GenericEventQueue() ;
        m_iconCache    = new ApplicationIconCache() ;

        // Add ourselves as a strategy listener
        Shell.getInstance().getDefaultDialingStrategy().
                addDialingStrategyListener(new icDialingStrategyListener()) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Determine the default user (displayname<user@domain:port>)
     */
    public String getDefaultUser()
    {
        String strDefaultUser = "" ;
        SipLineManager manager = SipLineManager.getInstance() ;

        SipLine line = manager.getDefaultLine() ;
        if (line != null)
        {
            strDefaultUser = line.getUserEnteredUrl();
        }

        return strDefaultUser ;
    }



    /**
     * Public main - Execution starts here.
     */
    public void main(String argv[])
    {
        // Pre-create all of our forms and let them hang around/be recycled
        m_formIdle            = new IdleForm(this) ;
        m_formWaitingForCall  = new WaitingForCallForm(this) ;
        m_formCallInProgress  = new CallInProgressForm(this) ;
        m_formIncomingCall    = new IncomingCallForm(this) ;
        m_formDialer          = new DialerForm(this) ;
        m_terminalListener    = new icTerminalListener() ;

        m_phoneState = PhoneState.getInstance() ;

        // Tell the initialize() method that they are ok to continue.
        m_semStartUp.up() ;

        // Wait for button events queued from our button up and button down
        // hooks.
        while (true)
        {
            try
            {
                Object event = m_eventqueue.getNextEvent() ;
                if (event instanceof PButtonEvent)
                {
                    processButtonEvent((PButtonEvent) event) ;
                }
                else if (event instanceof CoreEvent)
                {
                    processCoreEvent((CoreEvent) event) ;
                }
                else
                {
                    System.out.println("CoreApp:: Unknown event: " + event) ;
                }
            }
            catch (Exception e)
            {
                SysLog.log(e) ;
            }
        }
    }

    /**
     * Shown when call is in progress
     */
    public CallInProgressForm getCallInProgressForm()
    {
         return m_formCallInProgress;
    }

    /**
     * Ability to supply subclass for custom implementation
     */
    public void setCallInProgressForm(CallInProgressForm formCallInProgress)
    {
         m_formCallInProgress = formCallInProgress;
    }

    public IdleForm getIdleForm()
    {
        return m_formIdle ;
    }


    /**
     * Gets the application icon cache
     */
    public ApplicationIconCache getIconCache()
    {
        return m_iconCache ;
    }


    public void clearDialer()
    {
        if (m_formDialer.isStacked()) {
           m_formDialer.closeForm() ;
        }
    }


    public void doDial()
    {
        if (!m_formDialer.isInFocus()) {
            m_formDialer.showModeless() ;
        }
    }

    /**
     * Checks if any calls are on hold and removes the timer if possible
     * resets the timer if there are still calls on hold
     *
     * @since 1.1
     *
     *
     * @param none
     *
     * @return void
     * @author Daniel Winsor
     */
    public void checkOnHoldTimerState()
    {
        PCall heldCalls[] = Shell.getCallManager().getCallsByState(PCall.CONN_STATE_HELD) ;
        Timer holdtimer = Timer.getInstance();
        System.out.println("CHECKING HOLD TIMER STATE: heldCalls = " + heldCalls.length);
        if (heldCalls.length == 0)
        {
          //since this is the last one, remove the timers for this listener.
          holdtimer.removeTimers(m_coreListener);
          System.out.println("REMOVING HELD TIMER.");
        }
        else
        {
          //check if all held calls are ignored. If so then remove timers.
          boolean bNonIgnored = false;
          for (int loop = 0; loop < heldCalls.length;loop++)
          {
              if (!m_vIgnoreCalls.contains(heldCalls[loop].getCallID()))
              {
                 bNonIgnored = true;
                 break;
              }
          }

          if (bNonIgnored == true) //we found at least one so keep beeping
          {
              //just reset the timer
              int holdRemindTime = 30;
              holdRemindTime = Settings.getInt("HELD_CALL_REMINDER_PERIOD",holdRemindTime);
              holdtimer.resetTimer(holdRemindTime*1000, m_coreListener, null, true);
              System.out.println("RESETTING HELD TIMER.");
          }
          else
          {
              //they are all ignored so there is no need to beep.
              holdtimer.removeTimers(m_coreListener);
              System.out.println("REMOVING HELD TIMER.");
          }

        }
    }

    /**
     * When the hold button is pressed we will either place the current call
     * on hold, take the user to the release calls form or display and error
     * message.
     */
    public void doHold()
    {
        PCall call = Shell.getCallManager().getInFocusCall() ;

        if ((call != null) && call.getConnectionState() == PCall.CONN_STATE_IDLE)
        {
            try
            {
                call.disconnect() ;
            }
            catch (PCallException e)
            {
                SysLog.log(e) ;
            }
            call = Shell.getCallManager().getInFocusCall() ;
            try
            {
                Shell.getMediaManager().setAudioDevice(PAudioDevice.ID_NONE) ;
            }
            catch (PMediaLockedException e) { }

        }

        if (call == null)
        {
            onReleaseCall(false) ;
        }
        else
        {
            try
            {
                call.stopSound() ;
                call.stopTone() ;
            }
            catch (PSTAPIException e)
            {
                SysLog.log(e) ;
            }

            try
            {
                m_formIdle.beginWaitIcon() ;
                call.placeOnHold() ;

            }
            catch (PCallStateException cse)
            {
                displayPlaceCallOnHoldError() ;
                SysLog.log(cse) ;
            }
            catch (PCallException e)
            {
                SysLog.log(e) ;
            }
            finally
            {
                m_formIdle.endWaitIcon() ;
            }
        }
    }


    public void doOffHook()
    {
        if (DEBUG_METHODS)
            System.out.println("***** doOffHook() ENTER");
        Logger.post("hookswitch", Logger.TRAIL_COMMAND, "offHook") ;

        PCallManager callManager = Shell.getInstance().getCallManager() ;
        PCall call = callManager.getInFocusCall() ;

        if ((call != null) && (!m_vIgnoreCalls.contains(call.getCallID()))) {
            if (call.getConnectionState() == PCall.CONN_STATE_INBOUND_ALERTING) {
                doAnswerActiveCall(PAudioDevice.ID_HAND_SET) ;
            } else {
                try {
                    Shell.getMediaManager().setAudioDevice(PAudioDevice.ID_HAND_SET) ;
                } catch (PMediaException e) {
                    SysLog.log(e) ;
                }
            }
        }
        // We must be creating an outgoing call
        else  {
            if (call == null) {
                doCreateOutgoingCall(PAudioDevice.ID_HAND_SET) ;
            } else {
                try {
                    Shell.getMediaManager().setAudioDevice(PAudioDevice.ID_HAND_SET) ;
                } catch (PMediaException e) {
                    SysLog.log(e) ;
                }
            }
        }

        if (DEBUG_METHODS)
            System.out.println("******* doOffHook() EXIT");
    }


    public void doOnHook()
    {
        if (DEBUG_METHODS)
            System.out.println("******* doOnHook() ENTER");

        PMediaManager   mediaManager = Shell.getMediaManager() ;
        PCallManager callManager = Shell.getInstance().getCallManager() ;

        Logger.post("hookswitch", Logger.TRAIL_COMMAND, "onHook") ;

        if (mediaManager.getAudioDeviceID() == PAudioDevice.ID_HAND_SET) {
            doDropActiveCall() ;
        }

        if (mediaManager.getAudioDeviceID() == PAudioDevice.ID_NONE) {
            onIdle() ;
        }

        //if there is an incoming call, then put the Incoming call form back in focus
        PCall incomingCalls[] = callManager.getCallsByState(PCall.CONN_STATE_INBOUND_ALERTING);
        if ((incomingCalls != null) && (incomingCalls.length > 0))
        {
            if (DEBUG_EVENTS)
                System.out.println("******** INCOMING CALL FORM FOCUS *********");
            if (incomingCalls[0] != null)
            {
                PCall call = incomingCalls[0];
                PAddress address = call.getLocalAddress();
                m_formIncomingCall.onIncomingCall(incomingCalls[0], address, 0) ;
            }
        }

        if (DEBUG_METHODS)
            System.out.println("******* doOnHook() EXIT");
    }





    /**
     * Act like doHold(), except it will only take a call OFF hold and will
     * never put a call on hold.  It also always displays the select held
     * calls form
     */
    public void doReleaseCall()
    {
        if (DEBUG_METHODS)
            System.out.println("******* doReleaseCall() ENTER");
        onReleaseCall(true) ;
        if (DEBUG_METHODS)
            System.out.println("******* doReleaseCall() EXIT");
    }

  /** Shows a form to answer incoming ringing calls if
    * there are more than one incoming ringing call.
    * If there is just one incoming ringing call, it just answers
    * the call.
    *
    * After the form is shown, if OK is pressed,
    * if there is already a call which is in progress
    * ( that is being answered ), the call selected is put on hold
    * , otherwise it is answered.
    *
    */
    public void doAnswerRingingCall()
    {
        if (DEBUG_METHODS)
            System.out.println("******* doAnswerRingingCall() ENTER");
        onAnswerRingingCall() ;
        if (DEBUG_METHODS)
            System.out.println("******* doAnswerRingingCall() EXIT");
    }


    /**
     * shows up a SimpleTextForm box with sip errorcode and sip errortext.
     */
    public void doShowFailedCallInfo( PConnectionEvent failedEvent){

      final SimpleTextForm infoBox = new SimpleTextForm(this, "Failed Call Info") ;
      infoBox.getBottomButtonBar().clearItem(PBottomButtonBar.B2);
      PActionListener okListener = new PActionListener(){
                public void actionEvent(PActionEvent event){
                    if(  event.getActionCommand().equals(AppResourceManager.getInstance().getString("lblGenericOk"))){
                        infoBox.closeForm();
                    }
                }
            };


      PActionItem okActionItem = new PActionItem
            (new PLabel(AppResourceManager.getInstance().getString("lblGenericOk")),
             AppResourceManager.getInstance().getString("hint/xdk/simpletextform/ok" ),
             okListener,
             (AppResourceManager.getInstance().getString("lblGenericOk")));
      PActionItem[] actionItems = new PActionItem[1];
      actionItems[0] = okActionItem;
      infoBox.getBottomButtonBar().setItem(PBottomButtonBar.B3, okActionItem);
      infoBox.setLeftMenu(actionItems);

      String strCauseCode    =
                    failedEvent.getCauseAsString(failedEvent.getCause());
      String strSipResponse = "Not available";

      if( failedEvent.getResponseCode() != PConnectionEvent.DEFAULT_RESPONSE_CODE ){
        strSipResponse = failedEvent.getResponseText()+" ("+failedEvent.getResponseCode()+")";
      }

      StringBuffer strBuffer = new StringBuffer();
      strBuffer.append(AppResourceManager.getInstance().getString("lblFailedCallInfoPrompt1"));
      strBuffer.append("\n\nSIP code: ");
      strBuffer.append(strSipResponse);
      strBuffer.append("\nJTAPI code: ");
      strBuffer.append(strCauseCode);
      strBuffer.append("\n\n");
      strBuffer.append(AppResourceManager.getInstance().getString("lblFailedCallInfoPrompt2"));

      infoBox.setText( strBuffer.toString());
      infoBox.showModal();
    }


    /**
     * Transfer acts on the active call or the last call placed on hold.
     */
    public void doTransfer()
    {
        if (m_formManager.getActiveForm() != m_formDialer) {
            ApplicationManager appManager = ApplicationManager.getInstance() ;
            appManager.activateCoreApplication(ApplicationRegistry.TRANSFER_APP) ;
        }
    }


    /**
     *
     */
    public void doConference()
    {
        PCall  callInFocus = Shell.getCallManager().getInFocusCall() ;

        if (callInFocus == null || (callInFocus != null && callInFocus.getConnectionState() != PCall.CONN_STATE_INBOUND_ALERTING))
        {
            if (m_formManager.getActiveForm() != m_formDialer) {
                ApplicationManager appManager = ApplicationManager.getInstance() ;
                appManager.activateCoreApplication(ApplicationRegistry.CONFERENCE_APP) ;
            }
        }
    }


    public void displayCannotCreateCallError()
    {
        String msg = AppResourceManager.getInstance().getString("lblErrorMaxConnections") ;

        new icDisplayErrorThread(msg).start() ;
    }


    public void displayPlaceCallOnHoldError()
    {
        String msg = AppResourceManager.getInstance().getString("lblErrorCallHoldFailed") ;

        new icDisplayErrorThread(msg).start() ;
    }


    public void displayTakeCallOffHoldError()
    {
        String msg = AppResourceManager.getInstance().getString("lblErrorCallUnholdFailed") ;

        new icDisplayErrorThread(msg).start() ;
    }



    /**
     * Create an outgoing call and setup the the media/whatever.
     */
    public void doCreateOutgoingCall(int iMediaDevice)
    {
        if (DEBUG_METHODS)
            System.out.println("*************     COREAPP in doCreateOutgoingCall BEFORE executeHook ");
        Shell.getHookManager().executeHook(HookManager.HOOK_NEW_CALL, new NewCallHookData(iMediaDevice)) ;
        if (DEBUG_METHODS)
            System.out.println("*************     COREAPP in doCreateOutgoingCall AFTER executeHook ");
    }


    public void doDisplayDialer(String strInitial)
    {
        onDialing(strInitial) ;
    }


    public void sendDialingString(String strDialed)
    {
        m_formDialer.setDialString(strDialed) ;
    }


    public void funnelButtonEventToDialer(PButtonEvent event)
    {
        if (DEBUG_METHODS)
            System.out.println("*************     COREAPP in funnelButtonEventToDialer ENTER ");

        PCallManager callManager = Shell.getCallManager() ;
        PCall call = callManager.getInFocusCall() ;
        if (call == null) {
            try {
                call = PCall.createCall() ;
            } catch (PCallException e) {
                SysLog.log(e) ;

                MessageBox messageBox = new MessageBox(getApplication(), MessageBox.TYPE_ERROR) ;
                messageBox.setMessage(messageBox.getString("lblErrorMaxConnections")) ;
                messageBox.showModal() ;
            }
            m_formDialer.postFormEvent(event) ;
            m_formDialer.showModeless() ;

            try {
                Shell.getMediaManager().setDefaultAudioDevice() ;
            } catch (PMediaException e) {
                SysLog.log(e) ;
            }
        } else {
             m_formDialer.postFormEvent(event) ;
            if (!m_formDialer.isStacked())
                m_formDialer.showModeless() ;
        }

        if (DEBUG_METHODS)
            System.out.println("*************     COREAPP in funnelButtonEventToDialer EXIT ");
    }

    public PCall startDialer()
    {
        PCallManager callManager = Shell.getCallManager() ;
        PCall call = callManager.getInFocusCall() ;
        if (call == null) {
            try {
                call = PCall.createCall() ;
            } catch (PCallException e) {
                SysLog.log(e) ;

                MessageBox messageBox = new MessageBox(getApplication(), MessageBox.TYPE_ERROR) ;
                messageBox.setMessage(messageBox.getString("lblErrorMaxConnections")) ;
                messageBox.showModal() ;
            }

            if (call != null)
            {
                monitorCall(call) ;
            }

            try {
                Shell.getMediaManager().setDefaultAudioDevice() ;
            } catch (PMediaException e) {
                SysLog.log(e) ;
            }
        } else {
            monitorCall(call) ;
        }

        return call ;
    }



    /**
     * drop whatever the active call is
     */
    public void doDropActiveCall()
    {
        PCall call = Shell.getInstance().getCallManager().getInFocusCall() ;
        if (call != null) {
            try {
                Shell.getMediaManager().setAudioDevice(PAudioDevice.ID_NONE) ;
            } catch (PMediaException e) {
                SysLog.log(e) ;
            }
            hideActiveForm(call) ;
            try {
                call.disconnect() ;
            } catch (PCallException e) {
                SysLog.log(e) ;
            }
        }
    }


    /**
     * answer the active call.
     *
     * @TODO This code should really kick and scream if the active call is not
     *        an incoming call...
     */
    public void doAnswerActiveCall(int iMediaDevice)
    {
        PCall call = Shell.getInstance().getCallManager().getInFocusCall() ;
        if (call != null)
        {

            //
            // Stop ringing before answer.
            //
            try
            {
                call.stopSound() ;
            }
            catch (Exception e)
            {
                System.out.println(e) ;
            }

            //
            // Enable Audio
            //
            if (iMediaDevice == PAudioDevice.ID_NONE)
            {
                try
                {
                    Shell.getMediaManager().setDefaultAudioDevice() ;
                }
                catch (PMediaException e)
                {
                    SysLog.log(e) ;
                }
            }
            else
            {
                try
                {
                    Shell.getMediaManager().setAudioDevice(iMediaDevice) ;
                }
                catch (PMediaException e)
                {
                    SysLog.log(e) ;
                }
            }


            //
            // Answer
            //
            try
            {
                call.answer() ;

                // Force call into focus, but don't change actual
                // call leg states
                ((PtCallControlCall) call.getJTAPICall()).unhold(true) ;
            }
            catch (PCallException e)
            {
                SysLog.log(e) ;
            }
        }
    }


    /**
     *
     */
    public PtTerminalComponentListener getTerminalListener()
    {
        return m_terminalListener ;
    }


    /**
     * initialize the core application
     */
    public void initialize(FormManager formManager)
    {
        m_formManager = formManager ;

        // Wait for the core application to startup.
        m_semStartUp.down() ;

        // Display the Idle form
        m_formIdle.showModeless() ;

        // Update the Line Registration Feature Indicator
        m_fiLineRegistration = new LineRegistrationFeatureIndicator() ;
        m_fiLineRegistration.enable(true) ;
    }

    public void initializeCallState()
    {
        Shell.getInstance().getCallManager().addConnectionListener(new icCallStateMonitor()) ;
    }


    /**
     * This method is called explicitly in the PingtelEventDispatcher so that
     * the core application can handle specific button press before
     * applications.
     *
     * WARNING: Nobody should block this thread context or SEND an event to
     *          the event queue.  Doing so will cause a deadlock, as the
     *          system is waiting for this to complete before processing
     *          the next event.  This is why we are simply consuming the
     *          event below and firing it off to our own CoreApp context.
     */
    public void buttonDown(PButtonEvent event)
    {
        switch (event.getButtonID()) {
            case PButtonEvent.BID_VOLUME_UP:
            case PButtonEvent.BID_VOLUME_DN:
            case PButtonEvent.BID_SPEAKER:
            case PButtonEvent.BID_MUTE:
            case PButtonEvent.BID_HOLD:
            case PButtonEvent.BID_TRANSFER:
            case PButtonEvent.BID_CONFERENCE:
                m_eventqueue.postEvent(event) ;
                event.consume() ;
                break ;
            case PButtonEvent.BID_HEADSET:
                if (m_phoneState.isHeadsetAsHandset())
                {
                    // Consume; we send onhook/offhook events on the up
                    // depending on state.
                }
                else
                {
                    m_eventqueue.postEvent(event) ;
                }
                event.consume() ;

        }
    }


    /**
     * This method is called explicitly in the PingtelEventDispatcher so that
     * the core application can handle specific button press before
     * applications.
     *
     * WARNING: Nobody should block this thread context or SEND an event to
     *          the event queue.  Doing so will cause a deadlock, as the
     *          system is waiting for this to complete before processing
     *          the next event.  This is why we are simply consuming the
     *          event below and firing it off to our own CoreApp context.
     */
    public void buttonUp(PButtonEvent event)
    {
        ApplicationManager appManager = ApplicationManager.getInstance() ;
        PForm formActive = appManager.getActiveForm(this) ;
        boolean bModalDialogDisplaying = false ;
        if (formActive != null) {
            if (formActive.getDisplayState() == formActive.MODAL) {
                bModalDialogDisplaying = true ;
            }

            // If the task manger is showing, then strip out various buttons
            if (ShellApp.getInstance().isTaskManagerInFocus()) {
                switch (event.getButtonID()) {
                    case PButtonEvent.BID_HOLD:
                    case PButtonEvent.BID_TRANSFER:
                    case PButtonEvent.BID_CONFERENCE:
                    case PButtonEvent.BID_VOLUME_UP:
                    case PButtonEvent.BID_VOLUME_DN:
                        bModalDialogDisplaying = true ;
                        break ;
                }
            }
        }

        switch (event.getButtonID())
        {
            case PButtonEvent.BID_SPEAKER:
            case PButtonEvent.BID_MUTE:
            case PButtonEvent.BID_HOLD:
            case PButtonEvent.BID_TRANSFER:
            case PButtonEvent.BID_CONFERENCE:
                if (!bModalDialogDisplaying) {
                    m_eventqueue.postEvent(event) ;
                }
                event.consume() ;
                break ;
            case PButtonEvent.BID_VOLUME_UP:
            case PButtonEvent.BID_VOLUME_DN:
                if (!bModalDialogDisplaying) {
                    m_eventqueue.postEvent(event) ;
                }
                break ;
            case PButtonEvent.BID_POUND:
                if (m_bStateDebug) {
                    System.out.println("** FORM STACK **") ;
                    ShellApp.getInstance().m_formManager.dumpStack() ;
                    System.out.println("") ;
                    System.out.println("** CALL STACK **") ;
                    Shell.getCallManager().dump() ;
                }
                break ;
            case PButtonEvent.BID_HEADSET:
                if (!bModalDialogDisplaying)
                {

                    if (m_phoneState.isHeadsetAsHandset())
                    {
                        if (m_phoneState.isOnHook())
                        {
                            processOffHookNotification() ;
                        }
                        else
                        {
                            processOnHookNotification() ;
                        }
                    }
                    else
                    {
                        m_eventqueue.postEvent(event) ;
                    }
                }
                event.consume() ;
        }
    }


    /**
     * The specified button is being held down
     */
    public void buttonRepeat(PButtonEvent event)
    {

    }


    /**
     * Ignore the specified STAPI call.  In various situations, a developer
     * may want to monitor and react to state progressions itself-- instead of
     * allowing the core app to orchestrate the GUI.
     * <br>
     * A call is ignored by placing it's call id into vector.  Later, that vector
     * is compared whenever a state changes, but before an action it taken.
     */
    public synchronized void ignoreCall(PCall call)
    {
        ignoreCallByID(call.getCallID()) ;
    }


    /**
     * Ignore the specified STAPI call.  In various situations, a developer
     * may want to monitor and react to state progressions itself-- instead of
     * allowing the core app to orchestrate the GUI.
     * <br>
     * A call is ignored by placing it's call id into vector.  Later, that vector
     * is compared whenever a state changes, but before an action it taken.
     */
    public synchronized void ignoreCallByID(String strCallID)
    {
        ignoreCallByID(strCallID, true) ;
    }



    /**
     * Ignore the specified STAPI call.  In various situations, a developer
     * may want to monitor and react to state progressions itself-- instead of
     * allowing the core app to orchestrate the GUI.
     * <br>
     * A call is ignored by placing it's call id into vector.  Later, that vector
     * is compared whenever a state changes, but before an action it taken.
     */
    public synchronized void ignoreCallByID(String strCallID, boolean bHideForms)
    {
        if (!m_vIgnoreCalls.contains(strCallID))
        {
            System.out.println("~~~~~~~~~~~") ;
            System.out.println("IgnoreCall: " + strCallID) ;
            System.out.println("~~~~~~~~~~~") ;

            m_vIgnoreCalls.addElement(strCallID) ;
        }

        // Hide call even if already in list.
        if (bHideForms)
        {
            System.out.println("(*) Hiding form for: " + strCallID) ;
            hideActiveForm(strCallID) ;
        }


        // Refresh the hold timers
        checkOnHoldTimerState() ;

    }

    /**
     * Determine if the specified call has been held
     */
    public synchronized boolean isCallIgnored(PCall call)
    {
        return m_vIgnoreCalls.contains(call.getCallID()) ;
    }


    /**
     * Monitor a previously ignored call
     */
    public void monitorCall(PCall call)
    {
        monitorCallByID(call.getCallID()) ;
    }


    /**
     * Monitor a previously ignored call
     */
    public void monitorCallByID(String strCallID)
    {
        System.out.println("~~~~~~~~~~~") ;
        System.out.println("MonitorCall: " + strCallID) ;
        System.out.println("~~~~~~~~~~~") ;

        PCall call = Shell.getCallManager().getCallByCallID(strCallID) ;
        if (call != null)
        {
            // Kick out if the call has been dropped
            if (call.getJTAPICall() == null)
                return ;

            if (!Shell.getCallManager().isZombieCall(call)) {

                // Remove that call from the ignore list
                m_vIgnoreCalls.removeElement(strCallID) ;

                if (Shell.getCallManager().getInFocusCall() == call)
                {
                    if (Shell.getMediaManager().getAudioDeviceID() == PAudioDevice.ID_NONE)
                    {
                        try
                        {
                            Shell.getMediaManager().setDefaultAudioDevice(false) ;
                        } catch (Exception e) { }
                    }

                    switch (call.getConnectionState())
                    {
                        case PCall.CONN_STATE_IDLE:
                            onDialing(null) ;
                            break ;
                        case PCall.CONN_STATE_TRYING:
                            if (!m_formWaitingForCall.isInFocus())
                            {
                                hideActiveForm(call) ;
                                m_formWaitingForCall.showModeless() ;
                                PAddress participants[] = call.getParticipants() ;
                                m_formWaitingForCall.onTrying(call, participants[0]) ;
                            }
                            break ;
                        case PCall.CONN_STATE_FAILED:
                            if (!m_formWaitingForCall.isInFocus())
                            {
                                hideActiveForm(call) ;
                                m_formWaitingForCall.showModeless() ;

                                PAddress participants[] = call.getParticipants() ;
                                if((participants != null) && (participants.length > 0))
                                {
                                    m_formWaitingForCall.onFailed
                                        (call, participants[0], PConnectionEvent.CAUSE_UNKNOWN) ;
                                }else
                                {
                                    System.out.println(" call.getParticipants() in monitorCallByID when handling "+
                                     " PCall.CONN_STATE_FAILED returned a null or a zero length array ");
                                }
                            }
                            break ;
                        case PCall.CONN_STATE_CONNECTED:
                            if (call.isConferenceCall())
                            {
                                doConference() ;
                            }
                            else
                            {
                                PAddress participants[] = call.getParticipants() ;
                                if (participants.length > 0)
                                    displayCallStatus(call, participants[0]) ;
                            }
                            break ;
                        case PCall.CONN_STATE_OUTBOUND_ALERTING:
                            PAddress participants[] = call.getParticipants() ;
                            if (participants.length > 0)
                                displayCallStatus(call, participants[0]) ;
                            break ;
                    }
                }
            }
        }

        // If there are any calls on hold reset other if none then remove timer
        checkOnHoldTimerState() ;
    }



    /**
     * HACK: Users may need to monitor the dialer form.  For now, this proxy
     * acts a temp solution, I'm not sure what the correct interface/method
     * should be.  The dialer is abstracted away from user applications with
     * the dialing stategy and dial() methods on Shell.  Perhaps dial should
     * take some sort of dial listener or a form listener.
     *
     * @deprecated do not expose, yet.
     */
    public void addDialerFormListener(PFormListener listener)
    {
        m_formDialer.addFormListener(listener) ;
    }


    /**
     * HACK: Users may need to monitor the dialer form.  For now, this proxy
     * acts a temp solution, I'm not sure what the correct interface/method
     * should be.  The dialer is abstracted away from user applications with
     * the dialing stategy and dial() methods on Shell.  Perhaps dial should
     * take some sort of dial listener or a form listener.
     *
     * @deprecated do not expose, yet.
     */
    public void removeDialerFormListener(PFormListener listener)
    {
        m_formDialer.removeFormListener(listener) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    protected void processCoreEvent(CoreEvent event)
    {
    }

    protected void processButtonEvent(PButtonEvent event)
    {
        ApplicationManager appManager = ApplicationManager.getInstance() ;
        PCallManager callManager = Shell.getCallManager() ;
        PCall call = callManager.getInFocusCall() ;
        PMediaManager   mediaManager = Shell.getMediaManager() ;
        PForm           formActive ;

        if (event.getEventType() == PButtonEvent.BUTTON_UP) {

            switch (event.getButtonID()) {
                case PButtonEvent.BID_HOOKSWITCH:
                    if (!isUploadingFile())
                      doOffHook() ;
                    break ;
                case PButtonEvent.BID_VOLUME_UP:
                    formActive = m_formManager.getActiveForm() ;
                    if (!(formActive instanceof AdjustVolumeForm) && !(formActive instanceof SelectAdjustmentForm))
                    {
                        appManager.activateCoreApplication(ApplicationRegistry.VOLUME_ADJUST_APP, getActiveDevice()) ;
                    }
                    break ;
                case PButtonEvent.BID_VOLUME_DN:
                    formActive = m_formManager.getActiveForm() ;
                    if (!(formActive instanceof AdjustVolumeForm) && !(formActive instanceof SelectAdjustmentForm))
                    {
                        appManager.activateCoreApplication(ApplicationRegistry.VOLUME_ADJUST_APP, getActiveDevice()) ;
                    }
                    break ;
                case PButtonEvent.BID_SPEAKER:
                    {
                        if (call != null)
                        {
                            if (call.getConnectionState() == PCall.CONN_STATE_INBOUND_ALERTING)
                            {
                                doAnswerActiveCall(PAudioDevice.ID_SPEAKER_PHONE) ;
                            }
                            else if (mediaManager.getAudioDeviceID() == PAudioDevice.ID_SPEAKER_PHONE)
                            {
                                // If we have the phone off hook, then just switch to the handset...
                                if (!m_phoneState.m_bOnHook)
                                {
                                    try
                                    {
                                        mediaManager.setAudioDevice(PAudioDevice.ID_HAND_SET) ;
                                    }
                                    catch (PMediaException e)
                                    {
                                        SysLog.log(e) ;
                                    }
                                }
                                else
                                {
                                    doDropActiveCall() ;
                                    PCallManager manager = Shell.getCallManager() ;
                                    if (manager.getInFocusCall() == null)
                                    {
                                        onIdle() ;
                                    }
                                }
                            }
                            else
                            {
                                try
                                {
                                    mediaManager.setAudioDevice(PAudioDevice.ID_SPEAKER_PHONE) ;

                                    // If the phone is operating under the the headset as handset mode,
                                    // then pressing the speaker button will implicitly place the
                                    // handset on hook.
                                    if (m_phoneState.isHeadsetAsHandset() && !m_phoneState.isOnHook())
                                    {
                                        m_phoneState.setOnHook(true) ;
                                    }
                                }
                                catch (PMediaException e)
                                {
                                    SysLog.log(e) ;
                                }
                            }
                        }
                        else
                        {
                            if (DEBUG_EVENTS)
                                System.out.println("*************COREAPP BEFORE doCreateOutgoingCall ");
                            doCreateOutgoingCall(PAudioDevice.ID_SPEAKER_PHONE) ;
                            if (DEBUG_EVENTS)
                                System.out.println("*************COREAPP AFTER doCreateOutgoingCall ");
                        }
                    }
                    break ;
                case PButtonEvent.BID_MUTE:
                    PAudioDevice audioDevice = mediaManager.getAudioDevice() ;
                    if (audioDevice != null) {
                        try {
                            if (audioDevice.isMuted())
                            audioDevice.unmute() ;
                        else
                            audioDevice.mute() ;
                        } catch (PMediaException e) {
                            SysLog.log(e) ;
                        }
                    }
                    break ;
                case PButtonEvent.BID_HEADSET:
                    {
                        if (call != null) {
                            if (call.getConnectionState() == PCall.CONN_STATE_INBOUND_ALERTING) {
                                doAnswerActiveCall(PAudioDevice.ID_HEAD_SET) ;
                            } else if (mediaManager.getAudioDeviceID() == PAudioDevice.ID_HEAD_SET) {
                                // If we have the phone off hook, then just switch to the handset...
                                if (!m_phoneState.m_bOnHook) {
                                    try {
                                        mediaManager.setAudioDevice(PAudioDevice.ID_HAND_SET) ;
                                    } catch (PMediaException e) {
                                        SysLog.log(e) ;
                                    }
                                } else {
                                    doDropActiveCall() ;
                                    onIdle() ;
                                }
                            } else {
                                try {
                                    mediaManager.setAudioDevice(PAudioDevice.ID_HEAD_SET) ;
                                } catch (PMediaException e) {
                                    SysLog.log(e) ;
                                }
                            }
                        } else {
                            doCreateOutgoingCall(PAudioDevice.ID_HEAD_SET) ;
                        }
                    }
                    break ;
                case PButtonEvent.BID_HOLD:
                    doHold() ;
                    break ;
                case PButtonEvent.BID_TRANSFER:
                    doTransfer() ;
                    break ;
                case PButtonEvent.BID_CONFERENCE:
                    doConference() ;
                    break ;
            }
        } else if (event.getEventType() == PButtonEvent.BUTTON_DOWN) {
            switch (event.getButtonID()) {
                case PButtonEvent.BID_HOOKSWITCH:
                    doOnHook() ;
                    break ;
            }
        }
    }



    protected String getActiveDevice()
    {
        PMediaManager   mediaManager = Shell.getMediaManager() ;

        String VolumeDevice ;
        int selected_device = mediaManager.getAudioDeviceID() ;
        switch (selected_device)
        {
            case  PAudioDevice.ID_SPEAKER_PHONE:
                    VolumeDevice = Integer.toString(AdjustVolumeForm.BaseSpeaker);
                    break;
            case  PAudioDevice.ID_HAND_SET:
                    VolumeDevice = Integer.toString(AdjustVolumeForm.HandsetSpeaker);
                    break;
            case  PAudioDevice.ID_HEAD_SET:
                    VolumeDevice = Integer.toString(AdjustVolumeForm.HeadsetSpeaker);
                    break;
            case  PAudioDevice.ID_RINGER:
                    VolumeDevice = Integer.toString(AdjustVolumeForm.Ringer);
                    break;

            case  PAudioDevice.ID_NONE:
            default:
                VolumeDevice = Integer.toString(AdjustVolumeForm.Contrast);
        }

        return VolumeDevice ;
    }


    /**
     * Clear all internal caches of the specified call id.
     */
    private void clearCachesForCallID(String strCallID)
    {
        m_vIgnoreCalls.removeElement(strCallID) ;
        m_vZombieCalls.removeElement(strCallID) ;
    }

    /**
     * Display call status for the specified call/address.
     */
    public void displayCallStatus(PCall call, PAddress address)
    {
        PCall callInFocus = Shell.getCallManager().getInFocusCall() ;

        if ((call != null) && (callInFocus == call)) {
            if ((call.getConnectionState() == call.CONN_STATE_TRYING) ||
                (call.getConnectionState() == call.CONN_STATE_OUTBOUND_ALERTING))
            {
                if (!m_formWaitingForCall.isInFocus()) {
                    hideActiveForm(call) ;
                    m_formWaitingForCall.showModeless() ;
                }

                if (call.getConnectionState() == call.CONN_STATE_TRYING)
                {
                    m_formWaitingForCall.onTrying(call, address) ;
                }
                else
                {
                    m_formWaitingForCall.onRinging(call, address, PConnectionEvent.CAUSE_NORMAL) ;
                }

            } else {
                if (!m_formCallInProgress.isInFocus()) {
                    hideActiveForm(call) ;
                    m_formCallInProgress.showModeless() ;
                    try
                    {
                        Boolean bAutoEnable = (Boolean) call.getCallData("AUTO_ENABLE_DEVICE") ;
                        if ((bAutoEnable == null) || (bAutoEnable.booleanValue()))
                            Shell.getMediaManager().setDefaultAudioDevice() ;
                    }
                    catch (PSTAPIException e)
                    {
                        SysLog.log(e) ;
                    }
                }
                m_formCallInProgress.onCallInProgress(call, address) ;
            }
        }
    }



    private boolean isCoreModalFormInFocus()
    {
        boolean bRC = false ;

        PForm formInFocus = m_formManager.getActiveForm() ;

        if (    (formInFocus.getApplication() == this) &&
                ((formInFocus.getDisplayState() & PForm.MODAL) == PForm.MODAL)) {
            bRC = true ;
        }

        return bRC ;
    }




    /**
     *
     */
    private void onIdle()
    {
        if (!isCoreModalFormInFocus())
        {
            hideActiveForm() ;
            if (m_formManager != null)
            {
                PCall callInFocus = Shell.getCallManager().getInFocusCall() ;
                if ((callInFocus != null) && !m_vIgnoreCalls.contains(callInFocus.getCallID()))
                {
                    monitorCall(callInFocus) ;
                }
                else
                {
                    ApplicationManager appManager = ApplicationManager.getInstance() ;
                    appManager.activateApplication(Settings.getString("CORE_APPLICATION", "org.sipfoundry.sipxphone.sys.app.CoreApp")) ;
                }
            }
        }
    }


    /**
     *
     */
    private void onDialing(String strInitial)
    {
        PCall call = Shell.getCallManager().getInFocusCall() ;
        if (call == null) {
            try {
                call = PCall.createCall() ;
            } catch (PCallException e) {
                SysLog.log(e) ;
                Shell.getInstance().setDialingStrategy(null) ;
                displayCannotCreateCallError() ;
            }
        } else {
            Shell.getInstance().setDialingStrategy(null) ;
        }

        if (call != null) {
            m_formDialer.setInstructions(null) ;
            if (strInitial != null) {
                m_formDialer.setDialString(strInitial) ;
            }
            if (!m_formDialer.isInFocus()) {
                hideActiveForm(call) ;
                m_formDialer.showModeless() ;
            }
        }
    }

    /**
     * @param bAlwaysPrompt true to force the user to always select a call
     *        on hold or false to let the system decide (if 1 call, use it,
     *        otherwise prompt).
     */
    private void onReleaseCall(boolean bAlwaysPrompt)
    {
        PCall heldCalls[] = Shell.getCallManager().getCallsByState(PCall.CONN_STATE_HELD) ;
        PCall callRelease = null ;
        if (heldCalls.length > 0) {
            if ((heldCalls.length == 1) && !bAlwaysPrompt) {

                callRelease = heldCalls[0] ;


            } else {
                SelectHeldCallForm formReleaseCall = new SelectHeldCallForm(this, "Held Calls", "Release", AppResourceManager.getInstance().getString("hint/coreapp/release/release")) ;
                if (formReleaseCall.showModal() == SelectHeldCallForm.OK) {
                    callRelease = formReleaseCall.getSelectedCall() ;
                }
            }

            boolean bAnswerCall = false ;

            if (callRelease != null) {
                // ANSWER all alerting connections
                PAddress address[] = callRelease.getParticipants() ;
                for (int i=0; i<address.length; i++) {
                    if (callRelease.getConnectionState(address[i]) == PCall.CONN_STATE_INBOUND_ALERTING) {
                        bAnswerCall = true ;
                        break ;
                    }
                }

                try {
                    m_formIdle.beginWaitIcon() ;
                    callRelease.releaseFromHold() ;
                } catch (PCallStateException cse) {
                    displayTakeCallOffHoldError() ;
                    SysLog.log(cse) ;
                } catch (PCallException e) {
                    SysLog.log(e) ;
                } finally {
                    m_formIdle.endWaitIcon() ;
                }


                try {
                    Shell.getMediaManager().setDefaultAudioDevice() ;
                } catch (PMediaException e) {
                    SysLog.log(e) ;
                }

                if (bAnswerCall) {
                    try {
                        callRelease.answer() ;
                    } catch (PCallException e) {
                        SysLog.log(e) ;
                    }
                }
            }
        } else {
            System.out.println("!! No held calls!") ;
        }
    }

    /**
     * @param call call to release from hold
     */
    public void onReleaseCall(PCall callRelease)
    {
        if (callRelease != null) 
        {
            boolean bAnswerCall = false;

            // ANSWER all alerting connections
            PAddress address[] = callRelease.getParticipants();
            for (int i=0; i<address.length; i++) 
            {
                if (callRelease.getConnectionState(address[i]) == PCall.CONN_STATE_INBOUND_ALERTING) 
                {
                    bAnswerCall = true;
                    break;
                }
            }
            
            try 
            {
                m_formIdle.beginWaitIcon();
                callRelease.releaseFromHold();
            } 
            catch (PCallStateException cse) 
            {
                displayTakeCallOffHoldError();
                SysLog.log(cse);
            } 
            catch (PCallException e) 
            {
                SysLog.log(e);
            } 
            finally 
            {
                m_formIdle.endWaitIcon();
            }

            try 
            {
                Shell.getMediaManager().setDefaultAudioDevice();
            } 
            catch (PMediaException e) 
            {
                SysLog.log(e);
            }

            if (bAnswerCall) 
            {
                try 
                {
                    callRelease.answer();
                } 
                catch (PCallException e) 
                {
                    SysLog.log(e);
                }
            }
        }
    }


  /**
   * Shows a form to answer incoming ringing calls if
   * there are more than one incoming ringing call.
   * If there is just one incoming ringing call, it just answers
   * the call.
   *
   * After the form is shown, if OK is pressed,
   * if there is already a call which is in progress
   * ( that is being answered ), the call selected is put on hold
   * , otherwise it is answered.
   *
   */
    private void onAnswerRingingCall()
    {
        boolean bAlwaysPrompt = true;
        PCall incomingCalls[] =
            Shell.getCallManager().getCallsByState(PCall.CONN_STATE_INBOUND_ALERTING) ;
        PCall callToAnswer = null ;
        if (incomingCalls.length > 0) {
            if ((incomingCalls.length == 1) && !bAlwaysPrompt) {

                callToAnswer = incomingCalls[0] ;


            } else {
                SelectIncomingCallForm formAnswerCall = new SelectIncomingCallForm
                    (this,
                    "Incoming Calls",
                    "Answer",
                    AppResourceManager.getInstance().getString("hint/coreapp/incomingcall/answer")) ;

                if (formAnswerCall.showModal() == SelectIncomingCallForm.OK) {
                    callToAnswer = formAnswerCall.getSelectedCall() ;
                    PCall callInFocus = Shell.getCallManager().getInFocusCall() ;
                    if ((callInFocus != null) && (callInFocus != callToAnswer)) {
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
            }


            if (callToAnswer != null) {

                 // Answer, off hold, enable audio device

                try {
                    m_formIdle.beginWaitIcon() ;
                    callToAnswer.releaseFromHold() ;
                } catch (PCallStateException cse) {
                    displayTakeCallOffHoldError() ;
                    SysLog.log(cse) ;
                } catch (PCallException e) {
                    SysLog.log(e) ;
                } finally {
                    m_formIdle.endWaitIcon() ;
                }


                try {
                    Shell.getMediaManager().setDefaultAudioDevice() ;
                } catch (PMediaException e) {
                    SysLog.log(e) ;
                }

                try {
                    callToAnswer.answer() ;
                } catch (PCallException e) {
                    SysLog.log(e) ;
                }

            }
        } else {
            System.out.println("!! No incoming calls!") ;
        }
    }



    /**
     *
     */
    private void onReleaseCallFromHold(Call call)
    {
        if (call instanceof CallControlCall) {
            ((PtCallControlCall) call).unhold() ;
        }
    }


    private void hideForm(PForm form, PCall call)
    {
        String strCallID = null ;
        if (call != null)
            strCallID = call.getCallID() ;
        hideForm(form, strCallID) ;
    }

    private void hideForm(PForm form, String strCallID)
    {
        if ((form != null) && (m_formManager.isFormStacked(form))) {
            if (strCallID == null) {
                ((PAbstractForm) form).closeForm(0) ;
            } else {
                if (form instanceof CoreCallMonitor) {
                    PCall monitored = ((CoreCallMonitor) form).getMonitoredCall() ;
                    if ((monitored == null) || (monitored.getCallID().equals(strCallID))) {
                        ((PAbstractForm) form).closeForm(0) ;
                    }
                } else {
                    ((PAbstractForm) form).closeForm(0) ;
                }
            }
        }
    }



    /**
     * hide/destroy any active forms
     */
    private void hideActiveForm()
    {
        String strCallID = null ;

        hideActiveForm(strCallID) ;
    }


    /**
     * hide/destroy any active forms
     */
    private void hideActiveForm(PCall call)
    {

        String strCallID = null ;
        if (call != null)
            strCallID = call.getCallID() ;
        hideActiveForm(strCallID) ;

    }


    private void hideActiveForm(String strCallID)
    {
        hideForm(m_formWaitingForCall, strCallID) ;
        hideForm(m_formCallInProgress, strCallID) ;
        hideForm(m_formIncomingCall, strCallID) ;
        hideForm(m_formDialer, strCallID) ;
    }



    private void onDial(String strDial)
    {
        PCall call = Shell.getCallManager().getInFocusCall() ;
        if (call == null) {
            try {
                call = PCall.createCall() ;
                Shell.getMediaManager().setDefaultAudioDevice() ;
            } catch (PCallException e) {
                SysLog.log(e) ;
                displayCannotCreateCallError() ;
            } catch (PMediaException e) {
                SysLog.log(e) ;
            }
        }

        if (call != null) {
            try {
                call.connect(PAddressFactory.getInstance().createAddress(strDial)) ;
            } catch (PCallException e) {
                SysLog.log(e) ;
                displayCannotCreateCallError() ;
            }
        }
    }


    /**
     * Should the phone accept calls?  There are times when the phone should
     * be considered busy and thus not accept calls.  For example, during
     * phone startup and upgrades.
     *
     * @param bEnable True to enable incoming calls, false to disable.
     */
    public void setAcceptCalls(boolean bEnable)
    {
        m_bAcceptCalls = bEnable ;
    }

    /**
     * Should we accept incoming calls?
     *
     * @return true if we should accept calls, false if we should reject
     *         incoming calls.
     */
    public boolean isAcceptCallsEnabled()
    {
        return m_bAcceptCalls ;
    }

    /**
     * Is web server uploading a file?
     *
     *
     *
     * @param IsUploading true is in progress, or false if not.
     */
    public void setUploadingFile(boolean bIsUploading)
    {
        m_bUploadingFile = bIsUploading;

        setAcceptCalls(!bIsUploading);
    }

    /**
     * Are we uploading a file to the phone?
     *
     * @return true if we are uploading a file.
     */
    public boolean isUploadingFile()
    {
        return m_bUploadingFile;
    }


    /**
     * Invoked by the outside world whenever premium sound is disabled for a call.
     */
    public void premiumSoundDisabled(String strCallId)
    {
        if (m_formCallInProgress.isStacked())
            m_formCallInProgress.refreshCodecIndicator() ;
    }


    /**
     * Invoked when notified that the handset has been taken offhook
     */
    protected void processOffHookNotification()
    {
        m_phoneState.setOnHook(false) ;
        m_eventqueue.postEvent(new PButtonEvent(this, PButtonEvent.BUTTON_UP, PButtonEvent.BID_HOOKSWITCH)) ;
    }


    /**
     * Invoked when notified that the handset has been placed onhook
     */
    protected void processOnHookNotification()
    {
        m_phoneState.setOnHook(true) ;
        m_eventqueue.postEvent(new PButtonEvent(this, PButtonEvent.BUTTON_DOWN, PButtonEvent.BID_HOOKSWITCH)) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////
    private class icDialingStrategyListener implements DialingStrategyListener
    {
        /**
         * Called when a dialing attempt is made; we should close the dialing
         * form and bring up the the waiting for call form.
         */
        public void dialingInitiated(PCall call, PAddress address)
        {
            // hideForm(m_formDialer, (PCall) null) ;
        }


        /**
         * Called when a dialing attempt is aborted.  If the phone is off
         * hook, we should redisplay the dialing form, otherwise, we should
         * kill the call and close the dialing form.
         */
        public void dialingAborted(PCall call)
        {
            if (call != null)
            {
                hideForm(m_formWaitingForCall, call) ;
            }
            //show the dialer form if home screen is in focus and
            //if either handset is off-hook or speaker phone is on.
            if( ShellApp.getInstance().getCoreApp().getIdleForm().isInFocus() )
            {
                int device = Shell.getMediaManager().getAudioDeviceID();
                if ((device == PAudioDevice.ID_SPEAKER_PHONE || device == PAudioDevice.ID_HAND_SET ||
                     device == PAudioDevice.ID_HEAD_SET) 
                    && call != null && call.getConnectionState() == PCall.CONN_STATE_IDLE)
                {
                    ShellApp.getInstance().getCoreApp().doDial();
                }
            }
        }
    }

    /**
     * Terminal listener is created purely to listen and take action for/on
     * hook switch changes.
     *
     * We should probably listen for other events?
     */
    private class icTerminalListener extends PtTerminalComponentAdapter
    {
        public icTerminalListener()
        {
            super(IGNORE_VOLUME_EVENTS | IGNORE_BUTTON_EVENTS) ;
        }

        public void phoneHookswitchOffhook(PtTerminalComponentEvent event)
        {
            processOffHookNotification() ;
        }

        public void phoneHookswitchOnhook(PtTerminalComponentEvent event)
        {
            processOnHookNotification() ;
        }
    }


    public class icCommandDispatcher implements PActionListener
    {
        private void alertCallerOnHold()
        {
            PCall callInFocus = Shell.getCallManager().getInFocusCall() ;
            if (callInFocus != null)
            {
                try
                {
                    int iPlayLength = Settings.getInt("HELD_CALL_REMINDER_RING_LENGTH", 1050) ;
                    callInFocus.playTone(PtTerminalConnection.DTMF_TONE_CALLHELD, iPlayLength, true, false) ;
                }
                catch (PSTAPIException e)
                {
                    // ::TODO:: Handle this failure condition
                    SysLog.log(e) ;
                }
            }
            else
            {
                try
                {
                    int iPlayLength = Settings.getInt("HELD_CALL_REMINDER_RING_LENGTH", 1050) ;

                    // was doing this....but instead we copied the code below so we can catch the error
                    // mediaManager.playTone(PtTerminalConnection.DTMF_TONE_CALLHELD, iPlayLength) ;

                    try {
                        //no calls around so try and create one
                        PCall call = PCall.createCall() ;
                        Shell.getMediaManager().setDefaultAudioDevice(true) ;
                        //play call held tone
                        call.playTone(PtTerminalConnection.DTMF_TONE_CALLHELD, iPlayLength) ;
                        try {
                            call.stopTone() ;
                        }
                        catch (PSTAPIException e) {
                            SysLog.log(e) ;
                        }

                        try {
                            //disconnect the call used to play the tone
                            call.disconnect() ;
                        }
                        catch (PCallException e) {
                        SysLog.log(e) ;
                        }
                    }
                    catch (PCallException e) {
                        //if we made it here then all resources used to play sounds are in use
                        //we may want to initially create a resource on phone start
                        //so this exception never occurs.
                        System.out.println("*******************************************");
                        System.out.println("****** Could not play held call reminder.");
                        System.out.println("****** This error is most likely the result of having too many calls on hold.");
                        System.out.println("****** This will be fixed in a future version of the software.");
                        System.out.println("*******************************************");

                    }
                }

                catch (Exception e) {
                    System.out.println("Could not get the held call reminder ring length!");
                    // ::TODO:: Handle this failure condition
                    SysLog.log(e) ;
                }
            }
        }


        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(DialerForm.ACTION_DO_DIAL))
            {
                onDial((String) event.getObjectParam()) ;
            }
            else if (event.getActionCommand().equals(Timer.ACTION_TIMER_FIRED))
            {
                // we got here because a timer was created when a call was placed on hold
                System.out.println("On Hold Event Fired!");
                alertCallerOnHold();
            }
        }
    }


    /**
     * The call state monitor sits around and listens call state changes.
     * Upon receiving a call state change, the monitor simple reacts by
     * updating GUI forms or what not.
     */
    public class icCallStateMonitor implements PConnectionListener
    {
        /**
         * A call has been created by the PCall.createCall() method.  The call is
         * in the idle state and the PAddress within the event object is not yet
         * valid.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void callCreated(PConnectionEvent event)
        {
            // Look for a null call id and YELL
            String strCallID = event.getCall().getCallID() ;
            if (strCallID == null) {
                System.out.println("((*)) NULL call id received in callCreated listener") ;
                Thread.dumpStack() ;
            }

            // React to state change if in focus and not ignoring
        }


        /**
         * A call has been destroyed and is no longer valid.  This is an
         * great time to remove listeners and close applications/forms that
         * are monitoring specific calls.
         *
         * @param event The {@link PConnectionEvent} object containing both the call
         *        and address objects associated with the state change, and a
         *        cause ID if a failure has occurred.
         */
        public void callDestroyed(PConnectionEvent event)
        {
            // Look for a null call id and YELL
            String strCallID = event.getCall().getCallID() ;
            if (strCallID == null) {
                System.out.println("((*)) NULL call id received in callDestroyed listener") ;
                Thread.dumpStack() ;
            }

          clearCachesForCallID(strCallID);

          //check if on hold and remove timer if needed
          checkOnHoldTimerState();

        }


        /**
         * A call/address connect() attempt has been made, however, the
         * target has not yet responded.  The state can proceed to outbound
         * alerting, connected, or failed.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void connectionTrying(PConnectionEvent event)
        {
            // Look for a null call id and YELL
            String strCallID = event.getCall().getCallID() ;
            if (strCallID == null) {
                System.out.println("((*)) NULL call id received in connectionTrying listener") ;
                Thread.dumpStack() ;
            }

            PCall callInFocus = Shell.getCallManager().getInFocusCall() ;
            PCall call = event.getCall() ;

            // React to state change if in focus and not ignoring
            if ((callInFocus == call) || (event.getCause() == event.CAUSE_TRANSFER))
            {
                if (!m_vIgnoreCalls.contains(strCallID))
                {
                    if (!m_formWaitingForCall.isInFocus())
                    {
                        hideActiveForm(callInFocus) ;
                        m_formWaitingForCall.showModeless() ;
                    }

                    m_formWaitingForCall.setTransfering(event.getCause() == event.CAUSE_TRANSFER) ;
                    if (callInFocus != null)
                    {
                        callInFocus.m_bTransfereeRelated = true ;
                    }

                    m_formWaitingForCall.onTrying(callInFocus, event.getAddress()) ;
                }
            }
        }


        /**
         * A call/address connect() has been acknowledged by the target and is
         * alerting or (ringing).  The state can proceed to disconnected,
         * connected, or failed.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void connectionOutboundAlerting(PConnectionEvent event)
        {
            // Look for a null call id and YELL
            String strCallID = event.getCall().getCallID() ;
            if (strCallID == null) {
                System.out.println("((*)) NULL call id received in connectionOutboundAlerting listener") ;
                Thread.dumpStack() ;
            }

            // React to state change if in focus and not ignoring
            PCall callInFocus = Shell.getCallManager().getInFocusCall() ;
            if (callInFocus == event.getCall()) {
                if (!m_vIgnoreCalls.contains(strCallID))
                {
                    if (!m_formWaitingForCall.isInFocus())
                    {
                        hideActiveForm(callInFocus) ;
                        m_formWaitingForCall.showModeless() ;
                    }

                    // If the early media is detected (SIP 183), then note that
                    // within the call.  We use this when determining how to answer
                    // the call in the CoreApp (order of operations).
                    if (event.getCause() == PConnectionEvent.CAUSE_UNKNOWN)
                    {
                        event.getCall().setCallData("early_media", new Boolean(true)) ;
                    }

                    m_formWaitingForCall.onRinging(event.getCall(), event.getAddress(),event.getCause()) ;
                }
            }
        }


        /**
         * An inbound call has been detected and is ringing the local terminal.
         * The user should pick up the phone to answer or can progamatically answer
         * the call by calling answer() on the call object held within the event
         * object.  The state can proceed to connected, failed, or disconnected.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void connectionInboundAlerting(PConnectionEvent event)
        {
            System.out.println("CoreApp :: connectionInboundAlerting") ;

            // Look for a null call id and YELL
            String strCallID = event.getCall().getCallID() ;
            if (strCallID == null) {
                System.out.println("((*)) NULL call id received in connectionInboundAlerting listener") ;
                Thread.dumpStack() ;
            }

            // Make sure this is an inbound call
            PCall call = event.getCall() ;
            PAddress address = event.getAddress() ;

            // React to state change if in focus and not ignoring
            if (!m_vIgnoreCalls.contains(strCallID)) {
               if (!m_formIncomingCall.isInFocus()) {
                    hideActiveForm(event.getCall()) ;
                    m_formIncomingCall.showModeless() ;
                }

                m_formIncomingCall.onIncomingCall(call, address, event.getCause()) ;
            }
        }


        /**
         * A connection has been established.  The state can proceed to
         * Disconnected, held, or failed.
         *
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void connectionConnected(PConnectionEvent event)
        {
            // Look for a null call id and YELL
            String strCallID = event.getCall().getCallID() ;
            if (strCallID == null) {
                System.out.println("((*)) NULL call id received in connectionConnected listener") ;
                Thread.dumpStack() ;
            }

            // React to state change if in focus and not ignoring
            PCall callInFocus = Shell.getCallManager().getInFocusCall() ;
            if (callInFocus == event.getCall()) {
                if (!m_vIgnoreCalls.contains(strCallID)) {

                    // Tell call to shutup.
                    try {
                        callInFocus.stopSound() ;
                        callInFocus.stopTone() ;
                    } catch (PSTAPIException e) {
                        SysLog.log(e) ;
                    }

                    displayCallStatus(callInFocus, event.getAddress()) ;
                }
            }

        }


        /**
         * A connection has failed for one of various reasons.  See the cause
         * within the connection event for more information.  The state will not
         * change after this point.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void connectionFailed(PConnectionEvent event)
        {
            // Look for a null call id and YELL
            String strCallID = event.getCall().getCallID() ;
            if (strCallID == null) {
                System.out.println("((*)) NULL call id received in connectionFailed listener") ;
                Thread.dumpStack() ;
            }


            // React to state change if in focus and not ignoring
            PCall call = event.getCall() ;
            PCall callInFocus = Shell.getCallManager().getInFocusCall() ;
            if (!m_vIgnoreCalls.contains(strCallID))
            {
                // Check the global state of the call - it is possible that
                // the call is comprised of multiple connections.
                if (call.getConnectionState() == PCall.CONN_STATE_FAILED)
                {
                    // Display failure status if
                    //  The call is in focus and NOT (The call was incoming and not rejected)
                    if (    ((callInFocus == call) || (callInFocus == null)) &&
                            (!(call.isInbound() && (event.getCause() == event.CAUSE_BUSY)) ||
                            call.m_bTransfereeRelated)) {
                        if (m_formIncomingCall.isStacked() && (m_formIncomingCall.getMonitoredCall() == call)) {
                            m_formIncomingCall.onCallDropped(call, event );
                        } else {
                            if (!m_formWaitingForCall.isInFocus())
                            {
                                hideActiveForm(call) ;
                                m_formWaitingForCall.showModeless() ;
                            }
                            m_formWaitingForCall.onFailed(call, event );
                        }
                    } else {
                        try {
                            event.getCall().disconnect() ;
                        } catch (PSTAPIException e) {
                            SysLog.log(e) ;
                        }

                        hideActiveForm(event.getCall()) ;
                    }
                }
                else
                {
                    if (call == callInFocus)
                    {

                        monitorCall(call) ;
                    }
                }
            }
        }


        /**
         * A connection has been disconnected for one of various reasons.  See the
         * cause within the connection event for more information.  The state will
         * not change after this point.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void connectionDisconnected(PConnectionEvent event)
        {
            // Look for a null call id and YELL
            String strCallID = event.getCall().getCallID() ;
            if (strCallID == null) {
                System.out.println("((*)) NULL call id received in connectionDisconnected listener") ;
                Thread.dumpStack() ;
            }

            // React to state change if in focus and not ignoring
            PCall call = event.getCall() ;
            PCall callInFocus = Shell.getCallManager().getInFocusCall() ;
            boolean bIgnoreCall = false  ;


            if (!m_vIgnoreCalls.contains(strCallID))
            {
                // Check the global state of the call - it is possible that
                // the call is comprised of multiple connections.
                if (call.getConnectionState() == PCall.CONN_STATE_DISCONNECTED)
                {

                    // Display failure status if
                    //  1) The call is in focus &&
                    //  2) The call was outgoing
                    //  3) The call was incoming and not rejected
                    if (    ((callInFocus == call) || (callInFocus == null)) &&
                            !(call.isInbound() && (event.getCause() == event.CAUSE_BUSY))) {

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

                        // Display hung up status if incoming or in progress
                        if (m_formIncomingCall.isStacked() && (m_formIncomingCall.getMonitoredCall() == call)) {
                            m_formIncomingCall.onCallDropped(call, event.getAddress(), event.getCause()) ;
                        } else if (m_formCallInProgress.isStacked() && (m_formCallInProgress.getMonitoredCall() == call)) {
                            m_formCallInProgress.onCallDropped(call, event.getAddress()) ;
                        } else if (m_formWaitingForCall.isStacked() && (m_formWaitingForCall.getMonitoredCall() == call)) {
                            m_formWaitingForCall.onFailed(call, event.getAddress(), event.getCause()) ;
                        } else {
                            hideActiveForm(call) ;
                        }
                    } else {
                        if (m_formIncomingCall.isStacked() && (m_formIncomingCall.getMonitoredCall() == call)) {
                            m_formIncomingCall.onCallDropped(call, event.getAddress(), event.getCause()) ;
                        } else if (m_formCallInProgress.isStacked() && (m_formCallInProgress.getMonitoredCall() == call)) {
                            m_formCallInProgress.onCallDropped(call, event.getAddress()) ;
                        } else {
                            hideActiveForm(call) ;
                        }
                    }
                }
                else
                {
                    if (call == callInFocus)
                    {
                        monitorCall(call) ;
                        bIgnoreCall = true ;
                    }
                }
            } else {
                bIgnoreCall = true ;
            }


            // Drop the call if we are still monitoring and the call state has progressed to disconencted
            if ((!bIgnoreCall) && (call != null) && call.isAutoDropEnabled())
            {
                try {
                    System.out.println("DISCONNECT: ") ;
                    call.dumpConnectionsCache() ;
                    call.dumpConnections() ;

                    if (event.getAddress().equals(call.getLocalAddress()) || (call.getConnectionState() <= PCall.CONN_STATE_DISCONNECTED))
                    {
                        // Disable media if the call in focus just dropped or if the call in focus is now null.
                        if ((call == callInFocus) || (callInFocus == null))
                        {
                            try
                            {
                                Shell.getMediaManager().setAudioDevice(PAudioDevice.ID_NONE) ;
                            }
                            catch (PMediaException e)
                            {
                                SysLog.log(e) ;
                            }
                        }

                        // Drop the call.
                        try
                        {
                            call.disconnect() ;
                        }
                        catch (PCallException e)
                        {
                            SysLog.log(e) ;
                        }
                    } else {
                        System.out.println("NOT DROPPING CALL: Call State appears non-disconnected.") ;
                        call.dumpConnections() ;
                        call.dumpConnectionsCache() ;
                    }
                } catch (Exception e) {
                    System.out.println("Error trying to drop call:") ;
                    SysLog.log(e) ;
                }
            }

            checkOnHoldTimerState();

            // The call in progress form and the incoming call form both
            // have a "Incoming Calls" button that may need to be updated.
            // (consider: an ringing inbound may have just been forwared
            // or hungup)
            if (call != callInFocus)
            {
                if (m_formCallInProgress.isStacked())
                    m_formCallInProgress.updateShowIncomingCallsButton() ;

                if (m_formIncomingCall.isStacked())
                    m_formIncomingCall.updateShowIncomingCallsButton() ;
            }
        }



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
        public void connectionUnknown(PConnectionEvent event)
        {
            boolean bIgnoreCall = false  ;

            // Look for a null call id and YELL
            String strCallID = event.getCall().getCallID() ;
            if (strCallID == null) {
                System.out.println("((*)) NULL call id received in connectionUnknown listener") ;
                Thread.dumpStack() ;
            }
        }


        /**
         * The entire call (all connections) have been placed on hold.  At this
         * point no audio will be exchanged between all participants of the call.
         * The state can change to connected, failed, or dropped from this point.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void callHeld(PConnectionEvent event)
        {
            // Look for a null call id and YELL
            String strCallID = event.getCall().getCallID() ;
            if (strCallID == null) {
                System.out.println("((*)) NULL call id received in callHeld listener") ;
                Thread.dumpStack() ;
            }

            // React to state change if in focus and not ignoring
            if (!m_vIgnoreCalls.contains(strCallID)) {
                hideActiveForm(event.getCall()) ;
            }

            if (event.getCall() == Shell.getCallManager().getInFocusCall() ||
                    (Shell.getCallManager().getInFocusCall() == null)) {
                try {
                    Shell.getMediaManager().setAudioDevice(PAudioDevice.ID_NONE) ;

                    PCall call = Shell.getCallManager().getCallByCallID(strCallID) ;
                    if (!Shell.getCallManager().isZombieCall(call))
                    {
                        //start the timer which fires events to play a beep that a call is on hold.
                        Timer holdtimer = Timer.getInstance();
                        int holdRemindTime = 30;
                        holdRemindTime = Settings.getInt("HELD_CALL_REMINDER_PERIOD",holdRemindTime);
                        //remove all previous timers
                        holdtimer.removeTimers(m_coreListener);
                        //now add the timer
                        holdtimer.addTimer(holdRemindTime*1000, m_coreListener, null, true);
                        System.out.println("HELD TIMER STARTED!");

                    }
                } catch (PMediaException e) {
                    SysLog.log(e) ;
                }
            }
        }


        /**
         * A call has been released (taken off) hold.  At this point audio is once
         * again be exchanged between all participants of the call.  The state is
         * connected and proceed to disconnected, failed, or dropped from this
         * point.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void callReleased(PConnectionEvent event)
        {
            // Look for a null call id and YELL
            String strCallID = event.getCall().getCallID() ;
            if (strCallID == null)
            {
                System.out.println("((*)) NULL call id received in callReleased listener") ;
                Thread.dumpStack() ;
            }

            // React to state change if in focus and not ignoring
            if (!m_vIgnoreCalls.contains(strCallID))
            {
                PAddress addess = null ;
                PCall call = event.getCall() ;
                PAddress participants[] = call.getParticipants() ;
                if (participants.length > 0)
                {
                    displayCallStatus(event.getCall(), participants[0]) ;
                }
            }
            checkOnHoldTimerState() ;
        }
    }


    public Application getApplication()
    {
        return this ;
    }

    protected class icDisplayErrorThread extends Thread
    {
        String m_strMessage ;

        public icDisplayErrorThread(String strMessage)
        {
            m_strMessage = strMessage ;
        }

        public void run()
        {
            MessageBox messageBox = new MessageBox(getApplication(), MessageBox.TYPE_ERROR) ;
            messageBox.setMessage(m_strMessage) ;
            messageBox.showModal() ;
        }
    }
}
