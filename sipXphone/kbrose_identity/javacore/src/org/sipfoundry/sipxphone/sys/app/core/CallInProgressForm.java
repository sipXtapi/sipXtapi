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
import java.util.Date ;
import java.text.DateFormat ;

import org.sipfoundry.util.SysLog;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.DateUtils;
import org.sipfoundry.telephony.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.util.AppResourceManager;
import org.sipfoundry.stapi.* ;

/**
 * This form is displayed after a call has been established and is now in
 * progress.  The form is additionally morphed into a caller hungup form.
 * <p>
 * NOTE: This form is recycled for different calls.  You may need to clear
 *       variables/states on FORM_CLOSED events.
 *
 * @author Robert J. Andreasen
 */
public class CallInProgressForm extends PApplicationForm
        implements CoreCallMonitor
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
    protected icCallDuration         m_durationTimer ;

    protected PLabel                 m_lblStatus ;
    protected PLabel                 m_lblSoundFeaturesIndicator ;  // Icon for sound features
    protected boolean                m_bSoundFeaturesIndicatorEnabled ; // Is the indicator on?
    protected PCall                  m_call ;
    protected Application            m_application;
    protected SoundFeaturesForm      m_soundFeaturesForm = null;
    protected SendDtmfPrompt         m_sendDtmfPrompt;


    /**
     * the delay in closing a "hung up", "Call Disconnected" or
     * "Caller Redirected" form
     */
    protected static int m_iCloseFormDelay =
                XpressaSettings.getHungUpFormCloseDelay();

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Default Constructor
     */
    public CallInProgressForm(Application application)
    {
        super(application, "Call In Progress") ;
        setFormName("Call In Progress");
        m_application = application;

        m_call = null ;
        initializeComponents() ;

        addButtonListener(m_DTMFGenerator) ;

        setHelpText(getString("call_in_progress"), getString("call_in_progress_title")) ;
        setTimeDateTitle(true) ;

        //create the features form, just don't show it yet.
        m_soundFeaturesForm = new SoundFeaturesForm(getApplication());
    }


    /**
     * initialize our status to "" whenever the form is initially shown
     */
    public synchronized int showModal()
    {
        if (m_lblStatus != null)
            m_lblStatus.setText("Status: Call in Progress...") ;

        initializeInProgressState() ;

        return super.showModal() ;
    }


    /**
     * initialize our status to "Call in Progress..." whenever the form is initially shown
     */
    public synchronized boolean showModeless()
    {
        if (m_lblStatus != null)
            m_lblStatus.setText("") ;

        initializeInProgressState() ;

        return super.showModeless() ;
    }


    /**
     * Initialize the GUI components/containers for this form.
     */
    protected void initializeComponents()
    {
        m_container = new PContainer() ;
        m_container.setLayout(null) ;

        m_lblStatus = new PLabel() ;
        m_lblStatus.setAlignment(PLabel.ALIGN_WEST) ;
        m_container.add(m_lblStatus) ;

        // Duration Timer
        m_durationTimer = new icCallDuration() ;
        m_container.add(m_durationTimer) ;

        addToDisplayPanel(m_container, new Insets(1,1,1,1)) ;

        m_lblSoundFeaturesIndicator = new PLabel(getImage("imgSoundFeatures")) ;


    }


    /**
     * Layout or "place" our components on the form's container.
     */
    public void doLayout()
    {
        Dimension   dimSize = getSize() ;

        super.doLayout() ;

        if (m_lblStatus != null)
            m_lblStatus.setBounds(0, 0, 150, 27) ;
        if (m_durationTimer != null)
            m_durationTimer.setBounds(0, 27, 150, 27) ;
        if (m_calleeRenderer != null)
            m_calleeRenderer.setBounds(1, 54, dimSize.width-4, 27) ;
    }


    /**
     * Invoked by the CoreApp to update the call information displayed on
     * this form.
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
     * Invoked by the Core Application (and internally) to place the form into
     * a "call in progress mode".  This method alters the buttons, title, and
     * status information.
     */
    public void onCallInProgress(PCall call, PAddress address)
    {
        //remove the timer that closes the form.
        Timer.getInstance().removeTimers(m_dispatcher);

        m_lblStatus.setText("Status: Call in Progress") ;
        setFormName("Call In Progress");
        m_call = call ;

        initializeInProgressState() ;

        updateCallerID(address) ;

        m_durationTimer.reset() ;
        m_durationTimer.doTick() ;
        updateShowIncomingCallsButton();

         m_soundFeaturesForm.enableGIPS(true) ;
    }


    /**
     * Invoked by the Core Application (and internally) to place the form into
     * a "caller hungup mode".  This method alters the buttons, title, and
     * status information.
     */
    public void onCallDropped(PCall call, PAddress address)
    {
        if( m_iCloseFormDelay > 0 )
        {
            Timer.getInstance().addTimer( m_iCloseFormDelay, m_dispatcher, null )  ;
        }

        //if the features form is open we should close it
        if (m_soundFeaturesForm != null)
            m_soundFeaturesForm.closeForm();

        if (call.getConnectionState() != PCall.CONN_STATE_CONNECTED)
        {

            m_lblStatus.setText("Status: Disconnected") ;
            setFormName("Disconnected");
            m_call = call ;

            initializeDisconnectedState() ;

            updateCallerID(address) ;

            m_durationTimer.stop() ;
            m_durationTimer.repaint() ;
            updateShowIncomingCallsButton();

        }
    }


    /**
     * called when the form is gaining focus
     *
     * @param formLosingFocus form that lost focus as a result of
     */
    public void onFocusGained(PForm formLosingFocus)
    {
        m_durationTimer.start() ;
        m_durationTimer.doTick() ;
        updateShowIncomingCallsButton();
    }


    /**
     * called when the form is losing focus
     *
     * @param formGainingFocus form that gain focus as a result of
     */
    public void onFocusLost(PForm formGainingFocus)
    {
        m_durationTimer.stop() ;
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
        cleanUpOnClose();
    }

    /**
     * catch the closeForm here and get ride of the soundFeatures form.
     *
     * @since 2.0.0
     *
     *
     * @param exitcode
     *
     * @return none
     * @exception none
     * @author D. Winsor
     */
    public void closeForm(int exitcode)
    {
        cleanUpOnClose();
        super.closeForm(exitcode);
    }

    private void cleanUpOnClose()
    {
        //if the features form is open we should close it
        if (m_soundFeaturesForm != null)
            m_soundFeaturesForm.closeForm();

        if (m_sendDtmfPrompt != null)
            m_sendDtmfPrompt.close();
    }

    public void closeForm()
    {
        cleanUpOnClose();
        super.closeForm();
    }


    public boolean promptToSendDtmf(String message)
    {
        m_sendDtmfPrompt = new SendDtmfPrompt(this);
        m_sendDtmfPrompt.setUpcomingDtmf(message);

        return m_sendDtmfPrompt.showModal();
    }


    /**
     * Invoked by the end user to close the form after the call has been
     * disconnected by the remote side.
     */
    public void onCancel()
    {
        closeForm() ;
    }


    /**
     * Invoked by the end user to hang up on the current caller and close the
     * form.
     */
    public void onDisconnect()
    {
        PCall call = Shell.getInstance().getCallManager().getInFocusCall() ;
        if (call != null)
        {

            closeForm() ;

            try {
                Shell.getMediaManager().setAudioDevice(PAudioDevice.ID_NONE) ;
            } catch (PMediaException e) {
                SysLog.log(e) ;
            }

            try {
                call.disconnect() ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
        }
    }

    /**
     * when "incoming calls" buton is pressed,
     * it shows a form of incoming calls where an
     * user can select a list of incoming ringing cals.
     */
    public void onShowIncomingCalls()
    {
        ShellApp.getInstance().getCoreApp().doAnswerRingingCall() ;
    }


    /**
     * Called when the user attempts to hang on the call.  Here, a handler
     * is invoked.
     */
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
        }

    }


    /**
     * Called when the user attempts to abort a call (consulative transfer).
     * Here, a handle is invoked.
     */
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
        }
    }


    /**
     * Called when the user would like to see the system about box.
     */
    public void onAbout()
    {
      SystemAboutBox box = new SystemAboutBox(getApplication());
      box.onAbout();
    }


    /**
     * Called when the user presses the "GIPS" icon to gather more information
     * about the codec.
     */
    public void onSoundFeaturesInformation()
    {
        m_soundFeaturesForm.showModal() ;
    }


    /**
     * Get the call being monitored by this CoreCallMonitor implementation.
     */
    public PCall getMonitoredCall()
    {
        return m_call ;
    }


    /**
     * Decide wether to show the "Incoming Calls" button or the codec feature
     * indicator.  The "Incoming calls" button will always trump the codec
     * feature indicator if the number of incoming calls is greater than 0.
     */
    public void updateShowIncomingCallsButton()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;
        PCall incomingCalls[] =
            Shell.getCallManager().getCallsByState(PCall.CONN_STATE_INBOUND_ALERTING) ;

        if (incomingCalls.length > 0)
        {
            PActionItem actionShowIncomingCalls = new PActionItem(
                    new PLabel("Incoming Calls"),
                    getString("hint/coreapp/incomingcall/show_incoming_calls"),
                    m_dispatcher,
                    m_dispatcher.ACTION_SHOW_INCOMING_CALLS) ;

            menuControl.setItem(PBottomButtonBar.B1, actionShowIncomingCalls) ;
            enableMenusByAction( true, m_dispatcher.ACTION_SHOW_INCOMING_CALLS);
        }
        else
        {
            enableMenusByAction(false, m_dispatcher.ACTION_SHOW_INCOMING_CALLS);
            if (isSoundFeaturesIndicatorEnabled())
            {
                PActionItem actionSoundFeatures = new PActionItem(
                        m_lblSoundFeaturesIndicator,
                        getString("hint/coreapp/incomingcall/codec_info"),
                        m_dispatcher,
                        m_dispatcher.ACTION_SOUNDFEATURES) ;

                menuControl.setItem(PBottomButtonBar.B1, actionSoundFeatures) ;
            }
            else
                menuControl.clearItem(PBottomButtonBar.B1) ;
        }
    }

    //called by PCall
    public void refreshCodecIndicator()
    {
        System.out.println("refresh Codecs");
        //we need to send out form and indication that the codec has changed
         m_soundFeaturesForm.enableGIPS(false) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////


    /**
     * Enable/Disable codec indicator.  If enabled, a icon will be displayed
     * in position B1 until disabled (or if mulitple inbound call are present).
     */
    protected void enableSoundFeaturesIndicator(boolean bEnable)
    {
        if (m_bSoundFeaturesIndicatorEnabled != bEnable)
        {
            m_bSoundFeaturesIndicatorEnabled = bEnable ;
            updateShowIncomingCallsButton() ;
        }
    }

    /**
     * Return whether the codec indicator is enabled or disabled.
     *
     * @return true if enabled otherwise false.
     */
    protected boolean isSoundFeaturesIndicatorEnabled()
    {
        return m_bSoundFeaturesIndicatorEnabled ;
    }


    /**
     * Generic Method for displaying error dialogs.
     */
    protected void displayErrorForm(String strErrorMsg,String strErrorTitle)
    {
        SimpleTextForm frmError = new SimpleTextForm(getApplication(), strErrorTitle) ;
        frmError.setText(strErrorMsg) ;
        frmError.showModal() ;

    }


    /**
     *
     */
    protected void initializeInProgressState()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;
        PMenuComponent   leftMenu = getLeftMenuComponent() ;
        PMenuComponent   rightMenu = getRightMenuComponent() ;
        PActionItem      actionHangup = null ;
        PActionItem      actionHangupAbort = null ;
        PActionItem      actionDisconnect = null;
        PActionItem      actionShowIncomingCalls = null;

        setFormName("Call In Progress");

        actionDisconnect = new PActionItem(new PLabel("Disconnect"),
                    getString("hint/coreapp/inprogress/disconnect"),
                    m_dispatcher,
                    m_dispatcher.ACTION_DISCONNECT) ;
        actionShowIncomingCalls = new PActionItem(new PLabel("Incoming Calls"),
                    getString("hint/coreapp/incomingcall/show_incoming_calls"),
                    m_dispatcher,
                    m_dispatcher.ACTION_SHOW_INCOMING_CALLS) ;

        // If we have a hangup handler, create a actionHangup ActionItem
        if (m_call != null)
        {
            ActionHandler handler = m_call.getHangupHandler() ;
            if (handler != null)
            {
                String strHangupHandlerLabel = handler.getLabel() ;
                String strHangupHandlerHint = handler.getHint() ;

                if (strHangupHandlerHint == null)
                {
                    strHangupHandlerHint = strHangupHandlerLabel ;
                }

                // Create Hangup handler
                actionHangup = new PActionItem(new PLabel(strHangupHandlerLabel),
                        strHangupHandlerLabel,
                        m_dispatcher,
                        m_dispatcher.ACTION_HANGUP) ;

                // Create Hangup abort handler
                actionHangupAbort = new PActionItem(new PLabel("Disconnect"),
                        getString("hint/coreapp/inprogress/disconnect"),
                        m_dispatcher,
                        m_dispatcher.ACTION_HANGUP_ABORT) ;
            }
        }


        // Update Bottom Button Bar
        if (actionHangup != null)
        {
            menuControl.setItem(PBottomButtonBar.B2, actionHangupAbort) ;
            menuControl.setItem(PBottomButtonBar.B3, actionHangup) ;
        }
        else
        {
            menuControl.setItem(PBottomButtonBar.B2, actionDisconnect) ;
            menuControl.clearItem(PBottomButtonBar.B3) ;
        }

        PActionItem actionSoundFeatures = new PActionItem(
                        new PLabel("Audio Information"),
                        getString("hint/coreapp/incomingcall/codec_info"),
                        m_dispatcher,
                        m_dispatcher.ACTION_SOUNDFEATURES) ;

        // Update Left Menu
        leftMenu.removeAllItems() ;
        if (actionHangup != null)
        {
            leftMenu.addItem(actionHangup) ;
            leftMenu.addItem(actionHangupAbort) ;
            leftMenu.addItem(actionShowIncomingCalls);
        }
        else
        {
            leftMenu.addItem(actionDisconnect) ;
            leftMenu.addItem(actionShowIncomingCalls);
        }

        // Update Right Menu
        rightMenu.removeAllItems() ;
        rightMenu.addItem(new PActionItem( new PLabel("About"),
                                    getString("hint/core/system/aboutform"),
                                    m_dispatcher,
                                    m_dispatcher.ACTION_ABOUT)) ;

        enableMenusByAction(false, m_dispatcher.ACTION_SHOW_INCOMING_CALLS);

        if (m_call != null)
        {
            enableSoundFeaturesIndicator(true) ;
            leftMenu.addItem(actionSoundFeatures);
        }
        else
        {
            enableSoundFeaturesIndicator(false) ;
        }
    }


    /**
     *
     */
    protected void initializeDisconnectedState()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        // Clean up menu items
        menuControl.clearItem(PBottomButtonBar.B2) ;
        menuControl.clearItem(PBottomButtonBar.B3) ;

        PActionItem actionCancel = new PActionItem( new PLabel("Close"),
                getString("hint/core/system/closeform"),
                m_dispatcher,
                m_dispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, actionCancel) ;


        PActionItem items[] = new PActionItem[1] ;
        items[0] = new PActionItem( new PLabel("Close"),
                                    getString("hint/core/system/closeform"),
                                    m_dispatcher,
                                    m_dispatcher.ACTION_CANCEL) ;
        setLeftMenu(items) ;

    }

//////////////////////////////////////////////////////////////////////////////
// Inner / Nested classes
////



    public class icEventDispatcher implements PActionListener
    {
        public final String ACTION_CANCEL       = "action_cancel" ;
        public final String ACTION_DISCONNECT   = "action_disconnect" ;
        public final String ACTION_ABOUT        = "action_about" ;
        public final String ACTION_SOUNDFEATURES = "action_soundfeatures" ;
        public final String ACTION_HANGUP       = "action_hangup" ;
        public final String ACTION_HANGUP_ABORT = "action_hangup_abort" ;
        public final String ACTION_SHOW_INCOMING_CALLS = "action_show_incoming_calls" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_CANCEL))
            {
                onCancel() ;
            }
            else if (event.getActionCommand().equals(ACTION_DISCONNECT))
            {
                onDisconnect() ;
            }
            else if (event.getActionCommand().equals(ACTION_ABOUT))
            {
                onAbout() ;
            }
            else if (event.getActionCommand().equals(ACTION_HANGUP))
            {
                onActionHangup() ;
            }
            else if (event.getActionCommand().equals(ACTION_HANGUP_ABORT))
            {
                onActionHangupAborted() ;
            }
            else if (event.getActionCommand().equals(ACTION_SHOW_INCOMING_CALLS))
            {
                onShowIncomingCalls() ;
            }
            else if (event.getActionCommand().equals(Timer.ACTION_TIMER_FIRED))
            {
                onCancel() ;
            }
            else if (event.getActionCommand().equals(ACTION_SOUNDFEATURES))
            {
                //open sound features listform
                onSoundFeaturesInformation() ;
            }

        }
    }


    public class icCallDuration extends PComponent implements PActionListener
    {
        protected String  m_strDuration = "00:00:00" ;
        protected String  m_strStartTime = "" ;


        /**
         * default constructor
         */
        public icCallDuration()
        {
            this.setOpaque(false) ;
            this.enableDoubleBuffering(true) ;
        }


        public void start()
        {
            // Kill any timers if they already exist and start up a new one
            Timer.getInstance().resetTimer(1000, this, null, true) ;
        }



        public void stop()
        {
            // Kill any timers if they already exist and start up a new one
            Timer.getInstance().removeTimers(this) ;
        }


        public void reset()
        {
            m_strDuration = "" ;
        }


        public void doTick()
        {
            // PCall call = Shell.getCallManager().getInFocusCall() ;
            PCall tickCall = m_call;

            if( tickCall != null ){
                m_strDuration = DateUtils.getDurationAsString(tickCall.getDuration()*1000) ;
                Date startTime = tickCall.getStartTime() ;

                if (startTime != null)
                {
                    DateFormat df = DateFormat.getDateInstance(DateFormat.SHORT) ;
                    DateFormat tf = DateFormat.getTimeInstance(DateFormat.SHORT) ;

                    m_strStartTime = df.format(startTime) + " " + tf.format(startTime) ;
                }
            }

            this.repaint() ;
        }



        public void paint(Graphics g)
        {
            g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
            g.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT)) ;

            g.drawString("Duration: " + m_strDuration, 0, 10) ;
            g.drawString("Since: " +    m_strStartTime, 0, 20) ;
        }


        public void actionEvent(PActionEvent event)
        {
            doTick() ;
        }
    }
}
