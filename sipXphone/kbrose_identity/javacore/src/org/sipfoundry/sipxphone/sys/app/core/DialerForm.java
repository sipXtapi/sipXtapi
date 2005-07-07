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
import org.sipfoundry.sip.SipLineManager;
import org.sipfoundry.sip.SipLine;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.sys.calllog.* ;

import org.sipfoundry.telephony.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.CoreApp ;
import org.sipfoundry.sipxphone.sys.appclassloader.ApplicationManager ;
import org.sipfoundry.sipxphone.service.* ;

import org.sipfoundry.stapi.* ;
import org.sipfoundry.stapi.event.* ;


/**
 * The Dialer form is sort of complicated.  It serves two purposes and
 * personalities:
 * <br><br>
 * <ol>
 *   <li>
 *     "Ready" Form, where the dialer text field and labels are displayed
 *     however, the same icons and look/feel of the idle form are present
 *   </li>
 *   <li>
 *     "Dialing" Form, where the edit controls are available for the dialer
 *   </li>
 * </ol>
 *
 * @author Robert J. Andreasen, Jr.
 */
public class DialerForm extends PApplicationForm
    implements DialingStrategyListener, CoreCallMonitor
{
    ////////////////////////////////////////////////////////////////////////
    // Constants
    ////
    /** action fired to listeners when time to dial */
    public static final String ACTION_DO_DIAL  = "action_do_dial" ;

    /** constants for m_iGUIState: "Ready" form state */
    private static final int STATE_READY    = 0 ;
    /** constants for m_iGUIState: "Dialer" form state */
    private static final int STATE_DIALER   = 1 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    /** Phone Number Text Field */
    private PDialField m_tfDial ;

    /** Action Button Bar */
    private PCommandBar m_bbActions ;

    /** container that houses the phone number text field and action button bar */
    private icDialerContainer m_contDialer ;

    /** dispatches commands to various onXXX handlers */
    private icCommandDispatcher m_commandDispatcher = new icCommandDispatcher();

    /** what is the title base of our form? */
    private String m_strTitle ;

    /** what state is the GUI in (STATE_READY || STATE_DIALER) */
    private int m_iGUIState = -1 ;

    /** what call are we working with, here? */
    private PCall m_call ;

    /** what is the current dialer strategy */
    private DialingStrategy m_dialerStrategy ;

    /** The initial Stategy used when creating the dialer */
    private DialingStrategy m_initialDialerStrategy;

    /** are we in the midst of dialing? */
    private boolean m_bDialing ;

    /** call monitor listener that is listening for call state changes */
    private icCallMonitor m_callMonitor ;

    /** Control where we can select the outbound user
     * m_lblOutboundUserValue and m_cbOutboundUser are mutually exclsuive
     */
    private PChoice m_cbOutboundUser ;

    /** lable to show outbound user value if there is only one line or
     *  if lines cannot be changed.
     *  m_lblOutboundUserValue and m_cbOutboundUser are mutually exclsuive.
     */
    private PLabel m_lblOutboundUserValue ;


    /** Label identifing the control to select the outbound user */
    private PLabel m_lblOutboundUser ;

    /** label for our phone number*/
    private PLabel m_lblDial ;

    /** optional instructions */
    private PLabel m_lblInstructions ;

    /* did we start a core app */
    private boolean bDialerOpenedCoreApp = false;
    /* and if we did, which one? */
    private int coreAppDialerOpened = -1;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Create a new instance of the dialer form
     */
    public DialerForm(Application application)
    {
        super(application, "Dialer") ;
        constructionHelper(application) ;

    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Sets the instructions displayed in this dialer form
     *
     * @param strInstructions instructions string or null to clear
     */
    public void setInstructions(String strInstructions)
    {
        if (m_contDialer != null) {
            m_contDialer.setInstructions(strInstructions) ;
        }
    }


    /**
     * called when the form is gaining focus
     *
     * @param formLosingFocus form that lost focus as a result of
     */
    public void onFocusGained(PForm formLosingFocus)
    {
        m_bDialing = false ;

        PCall callPrevious = m_call ;
        m_call = Shell.getInstance().getDialingStrategyCall() ;
        if (m_call == null)
            m_call = Shell.getCallManager().getInFocusCall() ;

        // We need to add a listener to monitor this call.  If any
        // connections move into trying, alerting, or established, we
        // need to close this form.  (The phonebook may have dialed).
        // Of course, this is slightly painful and requires a bit of
        // book keeping.

        // Do we need to unhook an old call?
        if ((callPrevious != null) && (callPrevious != m_call)) {
            callPrevious.removeConnectionListener(m_callMonitor) ;
        }

        // Do we need to hook a new call?
        if ((callPrevious != m_call) && (m_call != null)) {
            m_call.addConnectionListener(m_callMonitor) ;
        }
        applyDialingStrategy() ;

        m_tfDial.evaluateDialString() ;

        int OffHookRemindTime = 180; //default time in secs
        OffHookRemindTime = Settings.getInt("OFFHOOK_ALERT_TIMEOUT",OffHookRemindTime);

        /* start tone that indicates phone off hook */
        /* times in next func are in ms */

        new PlayMediaWhileCallState(m_call,PtTerminalConnection.DTMF_TONE_LOUDBUSY,
                            PCall.CONN_STATE_IDLE,
                            OffHookRemindTime*1000,OffHookRemindTime*1000);
    }


    /**
     * called when the form has lost focus.
     *
     * @param formGainingFocus form that is gaining focus over this form
     */
    public void onFocusLost(PForm formGainingFocus)
    {
        m_tfDial.clearTimers() ;
    }


    /**
     * called when the form is being closed.
     */
    public void onFormClosing()
    {
        // Since this form is recycled, it is best to explicitly null out
        // these references so they can be properly garbage collected.

        m_call = null ;
        m_dialerStrategy = null ;
    }


    /**
     * Update the title of the dialer form
     */
    public synchronized void updateTitle()
    {
        switch (m_iGUIState)
        {
            case STATE_READY:
                if ((m_strTitle != null) && (m_strTitle.length() > 0))
                    setTitle(m_strTitle) ;
                else
                    setTitle(getString("lblReadyFormTitle")) ;
                break ;
            default:
                if ((m_strTitle != null) && (m_strTitle.length() > 0))
                    setTitle(m_strTitle + ": " + getString("lblDialerFormTitle")) ;
                else
                    setTitle(getString("lblDialerFormTitle")) ;
                break ;
        }
    }


    /**
     * Invoked when the user presses the clear button.
     */
    public void onClear()
    {
        m_tfDial.setText("") ;
        m_tfDial.clear() ;
    }


    /**
     * Set the dialing string displayed in the text field
     */
    public void setDialString(String strText)
    {
        m_tfDial.setText(strText) ;
        if ((strText != null) && (strText.length() > 0))
        {
            initDialerForm() ;
        }
    }


    /**
     * Invoked when the user presses the dial button.
     */
    public synchronized void onDial()
    {
        if (!m_bDialing) {
            m_bDialing = true ;
            String strDialString = m_tfDial.getText() ;
            PBottomButtonBar menuControl = getBottomButtonBar() ;
            if (menuControl != null)
            {
            	menuControl.clearItem(PBottomButtonBar.B3) ;
            }
            int iNumOutboundLines = getOutboundLinesNum();
            //here, null basically means use the default user.
            PAddress addrLocalUser = null;
            if( iNumOutboundLines > 1 )
            {
                String  strLocalUser = null;
                
                if (m_cbOutboundUser != null)
                {
                	SipLine line = (SipLine)m_cbOutboundUser.getSelectedItem();
                	
                	if (line != null)
                	{
                		strLocalUser = line.getIdentity();
                	}
                }

                if( strLocalUser != null )
                {
                    try{
                        addrLocalUser = PAddressFactory.getInstance().createAddress(strLocalUser);
                    }catch (Exception e)
                    {
                        Shell.getInstance().showUnhandledException(e, false) ;
                    }
                }
            }

            try
            {
            	if (m_dialerStrategy !=null)
            	{
	                m_dialerStrategy.dial(
	                    addrLocalUser,
	                    PAddressFactory.getInstance().createAddress(strDialString),
	                    m_call
	                    ) ;
            	}
            }
            catch (Exception e)
            {
                Shell.getInstance().showUnhandledException(e, false) ;
            }
        }
    }


    /**
     * Invoked when the user presses the phonebook button.
     */
    public void onPhonebook()
    {
        switchToProxyDialerStrategy();
        ApplicationManager appManager = ApplicationManager.getInstance() ;

        // Clear our timers
        m_tfDial.clearTimers() ;
        m_tfDial.stopTones() ;

/*        if (    appManager.activateCoreApplication
                (ApplicationRegistry.PHONEBOOK_APP) &&

                appManager.waitForCoreApplication
                (ApplicationRegistry.PHONEBOOK_APP)) {

            if (m_call.getConnectionState() == PCall.CONN_STATE_IDLE) {
                onClear() ;
            }

        }
*/
    }


    /**
     * Invoked when the user presses the dial by url button.
     */
    public void onDialByURL()
    {
        switchToProxyDialerStrategy();
        ApplicationManager appManager = ApplicationManager.getInstance() ;
        PCall call = Shell.getCallManager().getInFocusCall() ;

        // Clear our timers
        m_tfDial.clearTimers() ;
        m_tfDial.stopTones() ;

        bDialerOpenedCoreApp = true;
        coreAppDialerOpened = ApplicationRegistry.DIAL_BY_URL_APP;

        if (    appManager.activateCoreApplication
                (ApplicationRegistry.DIAL_BY_URL_APP) &&

                appManager.waitForCoreApplication
                (ApplicationRegistry.DIAL_BY_URL_APP)) {


            if (m_call.getConnectionState() == PCall.CONN_STATE_IDLE) {
                onClear() ;
            }
        }

        bDialerOpenedCoreApp = false;
        coreAppDialerOpened = -1;

    }


    /**
     * Invoked when the user presses the speed dial button.
     */
    public void onSpeedDial()
    {
        switchToProxyDialerStrategy();
        ApplicationManager appManager = ApplicationManager.getInstance() ;
        PCall call = Shell.getCallManager().getInFocusCall() ;

        // Clear our timers
        m_tfDial.clearTimers() ;
        m_tfDial.stopTones() ;

        bDialerOpenedCoreApp = true;
        coreAppDialerOpened = ApplicationRegistry.SPEED_DIAL_APP;

        if (    appManager.activateCoreApplication
                (ApplicationRegistry.SPEED_DIAL_APP) &&

                appManager.waitForCoreApplication
                (ApplicationRegistry.SPEED_DIAL_APP)) {

            if (m_call.getConnectionState() == PCall.CONN_STATE_IDLE) {
                onClear() ;
            }
        }

        /* we don't have the core app open anymore so reset the flags that tell it to close
        /* on dialer shutdown */
        bDialerOpenedCoreApp = false;
        coreAppDialerOpened = -1;
    }


    /**
     * Invoked when the user presses the call log button
     */
    public void onCallLog()
    {
        switchToProxyDialerStrategy();
        ApplicationManager appManager = ApplicationManager.getInstance() ;
        PCall call = Shell.getCallManager().getInFocusCall() ;

        // Clear our timers
        m_tfDial.clearTimers() ;
        m_tfDial.stopTones() ;

        bDialerOpenedCoreApp = true;
        coreAppDialerOpened = ApplicationRegistry.CALL_LOG_APP;

        if (    appManager.activateCoreApplication
                (ApplicationRegistry.CALL_LOG_APP)&&

                appManager.waitForCoreApplication
                (ApplicationRegistry.CALL_LOG_APP)) {

            if (m_call.getConnectionState() == PCall.CONN_STATE_IDLE) {
                onClear() ;
            }
        }

        /* we don't have the core app open anymore so reset the flags that tell it to close
        /* on dialer shutdown */
        bDialerOpenedCoreApp = false;
        coreAppDialerOpened = -1;

    }


    /**
     * Invoked when the user presses the redial button
     */
    public void onRedial()
    {
        switchToProxyDialerStrategy();
        PAddress remoteAddress = CallLogManager.getInstance().getLastDialed();
        PAddress localAddress = CallLogManager.getInstance().getLocalAddressOfLastDialed();

        if (remoteAddress != null && localAddress != null) {
            m_dialerStrategy.dial(localAddress, remoteAddress) ;
        } else {
            getApplication().showMessageBox("Cannot Redial",
                    "No outgoing calls have been made on this phone since the last reset.") ;
            m_dialerStrategy.abort() ;
        }
    }


    /**
     * Invoked when the user presses the exit button
     */
    public void onCancel()
    {
        m_dialerStrategy.abort() ;
    }


    /**
     * Invoked when the user presses the move caret forward buttons
     */
    public void onCaretForward()
    {

        m_tfDial.moveCaretForward() ;
    }


    /**
     * Invoked when the user presses the move caret backward buttons
     */
    public void onCaretBackward()
    {
        m_tfDial.moveCaretBackward() ;
    }



    /**
     * Invoked when the user presses the backspace button
     */
    public void onBackspace()
    {
        m_tfDial.backspace() ;
    }


    /**
     * Get the call being monitored by this CoreCallMonitor implementation.
     */
    public PCall getMonitoredCall()
    {
        return m_call ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Common initialization across various constructors
     */
    private void constructionHelper(Application application)
    {
        setStringResourcesFile("DialerForm.properties") ;

        // Init Phone Number TextField
        m_tfDial = new PDialField() ;
        m_tfDial.setNumericMode(true);
        m_tfDial.addActionListener(m_commandDispatcher) ;
        m_tfDial.addTextListener(new icTextListener()) ;

        // Init Action ButtonBar
        m_bbActions = new PCommandBar() ;
        m_bbActions.addActionListener(m_commandDispatcher) ;

        // Create our display container
        m_contDialer = new icDialerContainer() ;
        m_contDialer.setBounds(0, 0, 100, 100) ;

        addToDisplayPanel(m_contDialer, new Insets(0, 0, 0, 0)) ;

        getBottomButtonBar().addActionListener(m_commandDispatcher) ;

        m_lblOutboundUser = new PLabel(getString("lblOutboundUser"),
                PLabel.ALIGN_WEST) ;

        m_cbOutboundUser = new PChoice(this,
                getString("lblSelectOutboundUserHint"),
                getString("lblSelectOutboundUserTitle")) ;

        m_cbOutboundUser.setAssociatedButton( PButtonEvent.BID_R4 );
        m_cbOutboundUser.setHelpText(getString("select_outbound_line")) ;
        m_cbOutboundUser.setItemRenderer(new DefaultSIPAddressRenderer());

        m_lblOutboundUserValue = new PLabel("", PLabel.ALIGN_WEST) ;
        m_lblOutboundUserValue.setWrapping(PLabel.WRAP_ELLIPSIS_POINTS) ;

        initReadyForm() ;
        setHelpText(getString("dialing_in_progress"),
                    getString("dialing_in_progress_title")) ;

        addFormListener(new icFormListener()) ;
        m_bDialing = false ;
        m_callMonitor = new icCallMonitor() ;
    }


    private int getOutboundLinesNum()
    {

        //the default for controlling lines if there are more than one
        //line is true.
        int iOutboundLinesNum =
            org.sipfoundry.sip.SipLineManager.getInstance().getLinesCount();
        String strControlPhonesetDialerOutboundLine =
            PingerConfig.getInstance()
                    .getValue(PingerConfig.PHONESET_DIALER_CONTROL_OUTBOUND_LINE) ;

        if( strControlPhonesetDialerOutboundLine == null )
        {
            strControlPhonesetDialerOutboundLine = "ENABLE";
        }
        else if(!(  strControlPhonesetDialerOutboundLine.equalsIgnoreCase("DISABLE") ||
                   strControlPhonesetDialerOutboundLine.equalsIgnoreCase("READONLY") ) )
        {
            strControlPhonesetDialerOutboundLine = "ENABLE";
        }


        //we need to find out if there is one or more than one outbound line.
        //if there is one outbound line, the default behaviour is to "NOT" show
        //the outbound line info. If there is more than one outbound line,
        //if the flag is READONLY, we only show one line.
        if( strControlPhonesetDialerOutboundLine.equalsIgnoreCase("ENABLE") ||
            strControlPhonesetDialerOutboundLine.equalsIgnoreCase("READONLY") )
        {
            if(  iOutboundLinesNum <= 1 )
            {
                String strShowSingleOutboundLine =
                    PingerConfig.getInstance()
                    .getValue(PingerConfig.PHONESET_DIALER_SHOW_SINGLE_OUTBOUND_LINE) ;
                if( strShowSingleOutboundLine == null )
                {
                    strShowSingleOutboundLine = "DISABLE";
                }
                else if(  !strShowSingleOutboundLine.equalsIgnoreCase("ENABLE")  )
                {
                    strShowSingleOutboundLine = "DISABLE";
                }

                if( ! strShowSingleOutboundLine.equalsIgnoreCase("ENABLE") )
                {
                    iOutboundLinesNum = 0;
                }
            }else
            {
                if( strControlPhonesetDialerOutboundLine.equalsIgnoreCase("READONLY") )
                {
                    iOutboundLinesNum = 1;
                }
            }
        }else
        {
            iOutboundLinesNum = 0;
        }
        return iOutboundLinesNum;
    }


    /**
     * Populate the list of possible outbound users
     */
    private synchronized void populateOutboundUsers()
    {
        if (m_cbOutboundUser != null)
        {
            m_cbOutboundUser.removeAll();

            //m_cbOutboundUser.setText(((CoreApp) getApplication()).getDefaultUser()) ;
            SipLineManager manager = SipLineManager.getInstance() ;

            // Get all of the lines and sort them.
            SipLine lines[] = manager.getLines() ;
            //sortLines(lines, m_iSortMethod) ;
            SipLine defaultLine = manager.getDefaultLine();
            // Next add them to the list model.
            //TextUtils.debug("defaultLine is " + defaultLine );

            for (int i=0; i<lines.length; i++)
            {
                m_cbOutboundUser.addItem(lines[i]) ;
                if( lines[i].equals(defaultLine) )
                {
                    m_cbOutboundUser.setSelectedIndex(i);
                }
            }
        }

        if (m_lblOutboundUserValue != null)
        {
            m_lblOutboundUserValue.setText(((CoreApp) getApplication()).getDefaultUser()) ;
        }
    }


    /**
     * Update the GUI to 'be' the "Ready" form.  This involves tweaking the
     * menu and command bar buttons.
     */
    private synchronized void initReadyForm()
    {
        if (m_iGUIState != STATE_READY)
        {
            m_iGUIState = STATE_READY ;

            // Initialize Command Bar
            m_bbActions.removeAllButtons() ;

            m_bbActions.addButton
                (new PLabel(getImage("imgRedialIcon")),
                 m_commandDispatcher.ACTION_REDIAL,
                 getString("hint/coreapp/idle/redial")) ;

            if( Shell.getInstance().isSpeedDialEnabled())
            {
                m_bbActions.addButton(new PLabel(getImage("imgSpeedDialIcon")),
                                        m_commandDispatcher.ACTION_SPEEDDIAL,
                                        getString("hint/coreapp/idle/speeddial")) ;
            }
            if( Shell.getInstance().isCallLogEnabled())
            {
                m_bbActions.addButton(new PLabel(getImage("imgCallLogIcon")),
                                        m_commandDispatcher.ACTION_CALL_LOG,
                                        getString("hint/coreapp/idle/calllog")) ;
            }

            // Initialize Bottom Menu
            PBottomButtonBar menuControl = getBottomButtonBar() ;
            if( Shell.getInstance().isDialByUrlEnabled())
            {

                menuControl.setItem(PBottomButtonBar.B1,
                                    new PLabel("Dial by URL"),
                                    m_commandDispatcher.ACTION_DIAL_BY_URL,
                                    getString("hint/coreapp/idle/dialbyurl")) ;
            }

            menuControl.repaint() ;

            // Misc other stuff
            setHelpText(getString("dialing_in_progress"),
                        getString("dialing_in_progress_title")) ;
            updateTitle() ;

            // Init Menus
            String strAction = "Dial" ;
            String strHint=null;
            boolean bCanCancel = false ;
            if (m_dialerStrategy != null) {
                strAction = m_dialerStrategy.getAction() ;
                strHint = m_dialerStrategy.getActionHint() ;
                bCanCancel = m_dialerStrategy.isCancelable() ;
            }
            initMenus(strAction, strHint, bCanCancel) ;

            // Clear dial tone on next button press.
            m_tfDial.setClearDialTone() ;
        }
    }


    /**
     * Update the GUI to 'be' the "Dialer" form.  This involves tweaking the
     * menu and command bar buttons.
     */
    private synchronized void initDialerForm()
    {
        if (m_iGUIState != STATE_DIALER)
        {
            m_iGUIState = STATE_DIALER ;

            // Initialize Command Bar
            m_bbActions.removeAllButtons() ;
            m_bbActions.addButton
                (new PLabel(getImage("imgBackspaceIcon")),
                 m_commandDispatcher.ACTION_BACKSPACE,
                 getString("hint/core/edit/backspace")) ;

            m_bbActions.addButton
                (new PLabel(getImage("imgCharBackwardIcon")),
                 m_commandDispatcher.ACTION_BACKWARD,
                 getString("hint/core/edit/moveleft")) ;

            m_bbActions.addButton
                (new PLabel(getImage("imgCharForwardIcon")),
                 m_commandDispatcher.ACTION_FORWARD,
                 getString("hint/core/edit/moveright")) ;

            // Misc other stuff
            setHelpText(getString("dialing_in_progress"),
                        getString("dialing_in_progress_title")) ;
            updateTitle() ;

            // Init Menus
            String strAction = "Dial" ;
            String strHint = null ;
            boolean bCanCancel = false ;
            if (m_dialerStrategy != null) {
                strAction = m_dialerStrategy.getAction() ;
                strHint = m_dialerStrategy.getActionHint() ;
                bCanCancel = m_dialerStrategy.isCancelable() ;
            }
            initMenus(strAction, strHint, bCanCancel) ;
        }
    }


    public void initMenus(String strAction, String strHint, boolean bCanCancel)
    {
        PMenuComponent leftMenu = getLeftMenuComponent() ;
        PMenuComponent rightMenu = getRightMenuComponent() ;
        // Initialize right menus
        leftMenu.removeAllItems();
        rightMenu.removeAllItems();

        if (m_iGUIState == STATE_READY)
        {
            leftMenu.addItem( new PActionItem(new PLabel("Redial"),
                            getString("hint/coreapp/idle/redial"),
                            m_commandDispatcher,
                            m_commandDispatcher.ACTION_REDIAL)) ;

            if( Shell.getInstance().isSpeedDialEnabled())
            {
                leftMenu.addItem( new PActionItem(new PLabel("Speeddial"),
                                getString("hint/coreapp/idle/speeddial"),
                                m_commandDispatcher,
                                m_commandDispatcher.ACTION_SPEEDDIAL)) ;
            }

            if( Shell.getInstance().isCallLogEnabled())
            {
                leftMenu.addItem( new PActionItem(new PLabel("Call Log"),
                                getString("hint/coreapp/idle/calllog"),
                                m_commandDispatcher,
                                m_commandDispatcher.ACTION_CALL_LOG)) ;
            }

            if( Shell.getInstance().isDialByUrlEnabled())
            {
                leftMenu.addItem( new PActionItem(new PLabel("Dial by URL"),
                                getString("hint/coreapp/idle/dialbyurl"),
                                m_commandDispatcher,
                                m_commandDispatcher.ACTION_DIAL_BY_URL)) ;
            }

            rightMenu.addItem( new PActionItem(new PLabel("About"),
                            getString("hint/core/system/aboutform"),
                            m_commandDispatcher,
                            m_commandDispatcher.ACTION_ABOUT)) ;
            rightMenu.addItem( new PActionItem(new PLabel(strAction),
                            strHint,
                            m_commandDispatcher,
                            m_commandDispatcher.ACTION_DIAL) );

        }
        else if (m_iGUIState == STATE_DIALER)
        {
            if( Shell.getInstance().isDialByUrlEnabled())
            {
                leftMenu.addItem( new PActionItem(new PLabel(""),
                                "",
                                m_commandDispatcher,
                                m_commandDispatcher.ACTION_DIAL_BY_URL)) ;

                leftMenu.addItem( new PActionItem(new PLabel(""),
                                "",
                                m_commandDispatcher,
                                m_commandDispatcher.ACTION_DIAL_BY_URL)) ;

                leftMenu.addItem( new PActionItem(new PLabel(""),
                                "",
                                m_commandDispatcher,
                                m_commandDispatcher.ACTION_DIAL_BY_URL)) ;

                leftMenu.addItem( new PActionItem(new PLabel("Dial by URL"),
                                getString("hint/coreapp/idle/dialbyurl"),
                                m_commandDispatcher,
                                m_commandDispatcher.ACTION_DIAL_BY_URL)) ;

                leftMenu.enableItemAt(false,0); //dummy component
                leftMenu.enableItemAt(false,1); //dummy component
                leftMenu.enableItemAt(false,2); //dummy component
            }

            rightMenu.addItem( new PActionItem(new PLabel("About"),
                            getString("hint/core/system/aboutform"),
                            m_commandDispatcher,
                            m_commandDispatcher.ACTION_ABOUT)) ;

            rightMenu.addItem( new PActionItem(new PLabel(strAction),
                            strHint,
                            m_commandDispatcher,
                            m_commandDispatcher.ACTION_DIAL)) ;

        }
    }


    /**
     * This form can morph slightly based on the current dialing strategy.
     * This method applies those morphing changes
     */
    private void applyDialingStrategy()
    {
        //TextUtils.debug("applyDialingStrategy called");

        Dimension dimSize = m_contDialer.getSize() ;

        // Get the dialing Strategy
        m_dialerStrategy = Shell.getInstance().getDialingStrategy() ;
        m_dialerStrategy.addDialingStrategyListener(this) ;

        // Set the title
        m_strTitle = m_dialerStrategy.getFunction() ;
        if (m_strTitle == null)
            m_strTitle = "" ;
        updateTitle() ;

        // Set Instructions
        setInstructions(m_dialerStrategy.getInstructions()) ;

        // Initialize Menus
        initMenus(m_dialerStrategy.getAction(),
                m_dialerStrategy.getActionHint(),
                m_dialerStrategy.isCancelable()) ;

        // Initialize buttom button mar
        PBottomButtonBar menuControl = getBottomButtonBar() ;
        getBottomButtonBar().setItem(PBottomButtonBar.B3,
                            new PLabel(m_dialerStrategy.getAction()),
                            m_commandDispatcher.ACTION_DIAL,
                            m_dialerStrategy.getActionHint()) ;

        // Enable/Disable Cancel button as appropriate
        if (m_dialerStrategy.isCancelable())
        {

            menuControl.setItem( PBottomButtonBar.B2,
                                 new PLabel("Cancel"),
                                 m_commandDispatcher.ACTION_CANCEL,
                                 getString("hint/core/system/cancelform")) ;
        } else
        {
            menuControl.clearItem(PBottomButtonBar.B2) ;
        }

        // Enable/Disable Dial button (as appropriate).
        enableDialAction() ;

        // If the outbound user is changeable, then we need to move
        // around the controls on the dialog a bit.

        if ( m_dialerStrategy.isOutboundUserChangeable() )
        {
            int iOutboundLinesNumToShow = getOutboundLinesNum();
            if( iOutboundLinesNumToShow > 1 )
            {
                if (!m_contDialer.containsComponent(m_cbOutboundUser))
                {
                    // Remove these components
                    m_contDialer.remove(m_lblInstructions) ;

                    // Re-Add these components
                    m_contDialer.add(m_lblOutboundUser) ;
                    m_contDialer.add(m_cbOutboundUser) ;

                    // Reset the bounds of impacted components
                    m_lblOutboundUser.setBounds(28, 54, dimSize.width-28, 27) ;
                    m_cbOutboundUser.setBounds(28, 54+27, dimSize.width-32, 27) ;
                }
            }
            else if( iOutboundLinesNumToShow == 1 )
            {
                if (!m_contDialer.containsComponent(m_lblOutboundUserValue))
                {
                    // Remove these components
                    m_contDialer.remove(m_lblInstructions) ;

                    // Re-Add these components
                    m_contDialer.add(m_lblOutboundUser) ;
                    m_contDialer.add(m_lblOutboundUserValue) ;

                    // Reset the bounds of impacted components
                    m_lblOutboundUser.setBounds(28, 54, dimSize.width-28, 27) ;
                    m_lblOutboundUserValue.setBounds(28, 54+27, dimSize.width-30, 27) ;
                }
            }
            else
            {
                if (m_contDialer.containsComponent(m_cbOutboundUser))
                {
                    // Remove these components
                    m_contDialer.remove(m_lblOutboundUser) ;
                    m_contDialer.remove(m_cbOutboundUser) ;

                    // Re-Add the Instructions Label
                    m_contDialer.add(m_lblInstructions) ;

                    // Reset the bounds of impacted components
                    m_lblInstructions.setBounds(28, 54, dimSize.width-30, dimSize.height-54) ;
                    m_lblInstructions.doLayout() ;
                }
            }

        }
    }


    /**
     * Make sure we close the dialing form on cancel
     */
    public void dialingAborted(PCall call)
    {
        if (m_dialerStrategy !=
            Shell.getInstance().getDefaultDialingStrategy()) {
            closeForm() ;
        }
    }


    /**
     * Make sure we close the dialing form on a call attempt
     */
    public void dialingInitiated(PCall call, PAddress address)
    {
        closeForm() ;
    }


    protected DialerForm getForm()
    {
        return this ;
    }

    public void closeForm(int iExitCode)
    {
        super.closeForm(iExitCode);

        if (bDialerOpenedCoreApp && coreAppDialerOpened != -1)
        {
            ApplicationManager appManager = ApplicationManager.getInstance() ;
            appManager.closeCoreApplication(coreAppDialerOpened);
        }

    }

    /**
     * This method enables/disables the "dial" label on any menus/bottom
     * button bars based on the state of the dial field.  If the dial field
     * has data then the action is enabled, otherwise if the dial field is
     * blank then the action is disabled.
     */
    protected void enableDialAction()
    {
        boolean bEnable =
           ((m_tfDial.getText() != null) && (m_tfDial.getText().length() > 0));

        enableMenusByAction(bEnable, m_commandDispatcher.ACTION_DIAL) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////

    /**
     * This call monitor watched for any hint of the call being killed and
     * detached and closes the dial form
     */
    private class icCallMonitor implements PConnectionListener
    {
        public void callHeld(PConnectionEvent event) { }
        public void callReleased(PConnectionEvent event)  { }
        public void callCreated(PConnectionEvent event) { }
        public void connectionDisconnected(PConnectionEvent event) { }
        public void connectionInboundAlerting(PConnectionEvent event) { }
        public void connectionTrying(PConnectionEvent event) { }
        public void connectionOutboundAlerting(PConnectionEvent event) { }
        public void connectionConnected(PConnectionEvent event) { }
        public void connectionFailed(PConnectionEvent event) { }
        public void connectionUnknown(PConnectionEvent event) { }

        public void callDestroyed(PConnectionEvent event)
        {
            event.getCall().removeConnectionListener(this) ;
            if (m_dialerStrategy != null)
                m_dialerStrategy.abort() ;
            closeForm() ;
        }
    }


    /**
     * The command dispatcher sits arounds and waits for action event.  Once
     * an action event is received, it is dispatched to the appropriate onXXX
     * handler.
     */
    private class icCommandDispatcher implements PActionListener
    {
        protected final String ACTION_BACKWARD  = "action_backward" ;
        protected final String ACTION_BACKSPACE = "action_backspace" ;
        protected final String ACTION_FORWARD   = "action_forward" ;
        protected final String ACTION_CANCEL    = "action_cancel" ;
        protected final String ACTION_DIAL      = "action_dial" ;


        public final String ACTION_ABOUT	= "action_about" ;
        public final String ACTION_DIAL_BY_URL	= "action_dial_by_url" ;
        public final String ACTION_REDIAL	    = "action_redial" ;
        public final String ACTION_CALL_LOG	    = "action_call_log" ;
        public final String ACTION_PHONEBOOK	= "action_phonebook" ;
        public final String ACTION_SPEEDDIAL	= "action_speed_dial" ;


        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_ABOUT)) {
                SystemAboutBox box = new SystemAboutBox(getApplication());
                box.onAbout();
            }
            if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            }
            else if (event.getActionCommand().equals(ACTION_DIAL)) {
                onDial() ;
            }
            else if (event.getActionCommand().equals(ACTION_FORWARD)) {
                onCaretForward() ;
            }
            else if (event.getActionCommand().equals(ACTION_BACKWARD)) {
                onCaretBackward() ;
            }
            else if (event.getActionCommand().equals(ACTION_BACKSPACE)) {
                onBackspace() ;
            }
            else if (event.getActionCommand().equals(ACTION_PHONEBOOK)) {
                onPhonebook() ;
            }
            else if (event.getActionCommand().equals(ACTION_DIAL_BY_URL)) {
                onDialByURL() ;
            }
            else if (event.getActionCommand().equals(ACTION_REDIAL)) {
                onRedial() ;
            }
            else if (event.getActionCommand().equals(ACTION_CALL_LOG)) {
                onCallLog() ;
            }
            else if (event.getActionCommand().equals(ACTION_SPEEDDIAL)) {
                onSpeedDial() ;
            }
            else if (event.getActionCommand()
                     .equals(PDialField.ACTION_DIAL_STRING_COMPLETE)) {
                onDial() ;
            }
        }
    }


    /**
     * The text listener sits around and awaits for the dial field to be
     * updated.  After the initial update, this listener invokes the
     * initDialerForm() method which morphs this form into the dialer form
     */
    private class icTextListener implements PTextListener
    {
        public void textValueChanged(PTextEvent event)
        {
            if (m_iGUIState == STATE_READY)
                initDialerForm() ;

            enableDialAction() ;
        }

        public void caretPositionChanged(PTextEvent event) { }
    }


    /**
     * The dialer container contains the label, dial field and optional
     * intructions.
     */
    private class icDialerContainer extends PContainer
    {
        public icDialerContainer()
        {
            m_lblDial = new PLabel(this.getString("lblPhoneNumber"),
                                   PLabel.ALIGN_WEST) ;
            m_lblInstructions = new PLabel() ;

            layoutComponents() ;
        }


        public void setInstructions(String strInstructions)
        {
            if (m_lblInstructions != null) {
                m_lblInstructions.setText(strInstructions) ;
            }
        }


        protected void layoutComponents()
        {
            this.setLayout(null) ;

            this.add(m_bbActions) ;
            this.add(m_lblDial) ;
            this.add(m_tfDial) ;
            this.add(m_lblInstructions) ;
        }

        public void doLayout()
        {
            Dimension dimSize = this.getSize() ;

            // Add the command bar
            if (m_bbActions != null) {
                m_bbActions.setBounds(0, 0, 28, dimSize.height) ;
                m_bbActions.doLayout() ;
            }


            // Add the Dial Field
            if (m_tfDial != null) {
                m_tfDial.doLayout() ;
            }

            // Add the "Phone Number:" Label
            if (m_lblDial != null) {
                m_lblDial.setBounds(28, 0, dimSize.width-28, 27) ;
                m_bbActions.doLayout() ;
            }

            // Add the Dial Field
            if (m_tfDial != null) {
                m_tfDial.setBounds(28, 27, dimSize.width-30, 27) ;
                m_tfDial.doLayout() ;
            }

            // Add the optional instructions
            if (m_lblInstructions != null) {
                m_lblInstructions.setBounds(28, 54, dimSize.width-30, dimSize.height-54) ;
                m_lblInstructions.doLayout() ;
            }
        }
    }


    /**
     * This form listeners listens for opening and closing events and kicks
     * the GUI into the correct states.
     */
    private class icFormListener extends PFormListenerAdapter
    {
        public void formOpening(PFormEvent event)
        {
            if ((m_tfDial.getText() != null) &&
                (m_tfDial.getText().length() > 0)) {
                initDialerForm() ;
            } else {
                initReadyForm() ;
            }
            populateOutboundUsers() ;
        }


        public void formClosing(PFormEvent event)
        {
            m_tfDial.clearTimers() ;
            if (m_dialerStrategy != null) {
                m_dialerStrategy.removeDialingStrategyListener(getForm()) ;
            }
            onClear() ;
        }
    }

    /**
     * Switches to proxy DialerStrategy which doesn't close the form
     * when aborted.
     * Switch only if the current strategy is already not proxy.
     * ( otherwise  m_initialDialerStrategy would be set to
     * proxy itself which is not desired. )
     */
    private  void switchToProxyDialerStrategy(){
        if(! (m_dialerStrategy instanceof icProxyDialerStrategy) ){
            m_initialDialerStrategy =
                Shell.getInstance().getDialingStrategy();
            m_dialerStrategy = new icProxyDialerStrategy();
            m_dialerStrategy.addDialingStrategyListener
                (new icProxyDialingStrategyListener()) ;
            Shell.getInstance().setDialingStrategy
            ( m_dialerStrategy,
              Shell.getInstance().getDialingStrategyCall() );
        }
    }
    /**
     * switches back to original DialerStrategy.
     */
    private void switchBackToOriginalDialerStrategy(){
        m_dialerStrategy =  m_initialDialerStrategy  ;
        m_dialerStrategy.addDialingStrategyListener(this) ;
        Shell.getInstance().setDialingStrategy
            ( m_dialerStrategy,
              Shell.getInstance().getDialingStrategyCall() );
    }

    /**
     * Proxy Dialing Strategy Listener designed to be added to
     * proxy Dialing Strategy. This listener implements
     * <code>dialingAborted</code> in a diffent way than usual.
     * We just switch back to the normal stratgey used and
     * <strong>do not close </strong> this form.
     */
    private class icProxyDialingStrategyListener
        implements DialingStrategyListener{

        /**
         * This is the only method we wanted to implement in our custom way
         * because we didn't want DialerForm to be closed when an applicaton
         * launched from DialerForm gets closed( when DialerForm itself was
         * launched by another app like TransferApp or ConferenceApp ).
         * When dialing gets aborted ,
         * then switch to the original DialingStrategy.
         */
        public void dialingAborted(PCall call)
        {
           switchBackToOriginalDialerStrategy();
        }


        /**
         * get the listeners from the DialerStrategy and for each of them,
         * execute their method "dilaingInitaited" so that the behaviour
         * from this proxy is same as the original one they are supposed to
         * have.
         */
        public void dialingInitiated(PCall call, PAddress address)
        {
            DialingStrategyListener[] arrayListeners =
                m_initialDialerStrategy.getDialingStrategyListeners();
            for( int i = 0; i<arrayListeners.length; i++ ){
                arrayListeners[i].dialingInitiated(call, address);
            }
        }

    }



    /**
     * This proxy DialerStrategy is used so that we can implement
     * the <code>dialingAborted(PCall call)</code> method in
     * our custom way in proxy listener which is called by
     * <code> abort() </code> method in AbstractDialingStrategy.
     *
     * The need of this proxy implementation came when we didn't
     * want this DialerForm to be closed when an application launched
     * from DialerForm gets closed( when DialerForm itself was launched
     * from another app like TransferApp, or ConferenceApp ).
     * But we wanted the rest of the behaviour same as defined by the
     * stratgey they are supposed to have.
     */
    private class icProxyDialerStrategy extends AbstractDialingStrategy{


        /**
         * 'Dial' or invoke the method
         */
        public void dial(PAddress address){
            m_initialDialerStrategy.dial( address );
        }

        /**
         * 'Dial' or invoke the method
         */
        public void dial(PAddress address, PCall call){
            m_initialDialerStrategy.dial( address, call );
        }

        /**
         * Gets the string based representation of this strategy.  This text is
         * typically prepended the the form name.
         */
        public String getFunction(){
            return m_initialDialerStrategy.getFunction() ;
        }

        /**
         *Gets the string based representation of the dialing strategy action.
         * This text is typically displayed as the B3 button label
         */
        public String getAction(){
            return m_initialDialerStrategy.getAction() ;
        }

        /**
         * Get the string based instructions for this strategy.  This text is
         * typically displayed in lower half of the "dialer" form.
         */
        public String getInstructions(){
            return m_initialDialerStrategy.getInstructions() ;
        }

        /**
         * Gets the hint text associatd with the dialing strategy action.
         * This hint text is typically displayed when the B3 buttons is
         * pressed and held down.
         */
        public String getActionHint(){
            return m_initialDialerStrategy.getActionHint();
        }


        /**
         * Get the cancel status for this dialing strategy.
         * @return boolean true if this operation/strategy can be canceled,
         *         otherwise false
         */
        public boolean isCancelable()
        {
            return m_initialDialerStrategy.isCancelable();
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
            return m_initialDialerStrategy.isOutboundUserChangeable() ;
        }
    }
}
