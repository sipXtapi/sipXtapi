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

import java.net.* ;
import java.awt.* ;
import java.util.* ;
import java.text.* ;

import javax.telephony.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.telephony.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.* ;
import org.sipfoundry.sipxphone.sys.calllog.* ;
import org.sipfoundry.sipxphone.service.* ;

import org.sipfoundry.sipxphone.sys.app.shell.* ;
import org.sipfoundry.sipxphone.service.Timer;

import org.sipfoundry.stapi.* ;
import org.sipfoundry.sipxphone.sys.util.*;
import org.sipfoundry.sipxphone.featureindicator.* ;

/**
 * The "idle form" or "home screen" is the default/first screen that
 * typical users will see.	It is the jumping point to many different
 * screens.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class IdleForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    // The default cycle period in seconds
    private static final int DEFAULT_CYCLE_LENGTH = 5 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** entity responsible for dispatching action events */
    protected icEventDispatcher m_dispatcher = new icEventDispatcher() ;

    /** Command Bar */
    protected PCommandBar		m_bbActions ;

    protected PLabel            m_lblStatus = new PLabel("", PLabel.ALIGN_EAST) ;

    protected PLabel            m_lblTips = new PLabel("", PLabel.ALIGN_SOUTH) ;

    FeatureIndicatorPanel       m_pnlFeatureIndicator ;

    protected String            m_strDialierHandoff ;
    protected PCall             m_callHandoff ;
    protected DTMFGenerator     m_generator = new DTMFGenerator() ;

    protected int               m_iCycleLength = DEFAULT_CYCLE_LENGTH ;

    /** is cycling to be done for the titlebar display? */
    protected boolean m_bCycle = true;
    /** order of the cycle the titlebar is currently displaying */
    protected int m_iCycleOrder = 0;
    /* vector to hold the cycle patterns for display in the titlebar */
    protected Vector m_vCyclePatterns ;
    /* number of cycles for display in the titlebar*/
    protected int m_iNumberOfCycles = 0;
    /* hashtable to hold the substitution strings and values */
    protected Hashtable m_hashCyclingSubstitutions = new Hashtable();
    /* has the timer for cycling the display in titlebar added? */
    protected boolean m_bCycleTimerAdded = false;
    /**
     * boolean to check if the error found( if any) when substituting strings for
     * titlebar display is reported. If it is reported once, we do not want to
     * report it again.
     */
    private boolean m_bTitlebarDisplayErrorReported = false;

    /**
     * instance of inner class icDateTimeUpdateCallback which updates
     * the time on the title bar after specified time.
     */
    icDateTimeUpdateCallback m_dateTimeUpdater ;

    /**
     * The identity of the user of the current line.
     */
    private String m_strIdentity ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor our form under the specified application context
     */
    public IdleForm(Application application)
    {
        super(application, "Home") ;


        initializeMenus() ;
        initializeCommandbar() ;
        initializeMenubar() ;

        m_pnlFeatureIndicator = new FeatureIndicatorPanel(this) ;
        layoutComponents() ;

        addButtonListener(new icButtonListener()) ;
        m_lblStatus.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_LARGE)) ;

        setHelpText(getString("home"), getString("home_title")) ;
        initializeTitleSettings() ;
    }



    /**
     * Initialize the command bar
     */
    protected void initializeCommandbar()
    {
        // COMMAND BAR
        m_bbActions = new PCommandBar() ;
        m_bbActions.addActionListener(m_dispatcher) ;


        // Redial
        PActionItem actionRedial =new PActionItem( new PLabel(getImage("imgRedialIcon")),
                getString("hint/coreapp/idle/redial"),
                null,
                m_dispatcher.ACTION_REDIAL) ;
        m_bbActions.addButton(actionRedial) ;

        // Speeddial
        if( Shell.getInstance().isSpeedDialEnabled() )
        {
            PActionItem actionSpeedDial = new PActionItem( new PLabel(getImage("imgSpeedDialIcon")),
                    getString("hint/coreapp/idle/speeddial"),
                    null,
                    m_dispatcher.ACTION_SPEED_DIAL) ;
            m_bbActions.addButton(actionSpeedDial) ;
        }

        // Call Log
        if( Shell.getInstance().isCallLogEnabled() )
        {
            PActionItem actionCallLog = new PActionItem(new PLabel(getImage("imgCallLogIcon")),
                    getString("hint/coreapp/idle/calllog"),
                    null,
                    m_dispatcher.ACTION_CALL_LOG) ;
            m_bbActions.addButton(actionCallLog) ;
        }

//        // LDAP Phonebook
//        PActionItem actionPhonebook = new PActionItem(new PLabel(getImage("imgPhonebookIcon")),
//                getString("hint/coreapp/idle/phonebook"),
//                null,
//                m_dispatcher.ACTION_PHONEBOOK) ;
//        m_bbActions.addButton(actionPhonebook) ;
    }


    /**
     * called when the form is gaining focus
     *
     * @param formLosingFocus form that lost focus as a result of
     */
    public void onFocusGained(PForm formLosingFocus)
    {
        m_pnlFeatureIndicator.rotateTip() ;
        m_strDialierHandoff = null ;

        initializeTitleSettings() ;
    }


    /**
     * called when the form is losing focus
     *
     * @param formGainingFocus form that gain focus as a result of
     */
    public void onFocusLost(PForm formGainingFocus)
    {
        if (m_strDialierHandoff != null) {
            ((CoreApp) getApplication()).sendDialingString(m_strDialierHandoff) ;
            m_strDialierHandoff = null ;
            m_callHandoff = null ;
        }
        cleanupTitlebar() ;
    }


    protected void layoutComponents()
    {
        PContainer container = new PContainer() ;
        container.setLayout(null) ;
        container.setOpaque(false) ;

//        m_lblStatus.setBounds(28, 0, 160-30, 27) ;
//        container.add(m_lblStatus) ;

        m_bbActions.setBounds(0, 0, 27, 108) ;
        container.add(m_bbActions) ;

//        m_lblTips.setBounds(30, 27, 160-32, 108-32) ;
//        container.add(m_lblTips) ;
        container.add(m_pnlFeatureIndicator) ;
        m_pnlFeatureIndicator.setBounds(30, 0, 160-32, 108-4) ;

        addToDisplayPanel(container, new Insets(0,0,0,0)) ;
    }


    /**
     * Get the command bar for this form
     */
    public PCommandBar getCommandBar()
    {
        return m_bbActions ;
    }


    /**
     * Initialize the menu bar
     */
    protected void initializeMenubar()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        // Dial by URL
        if( Shell.getInstance().isDialByUrlEnabled() )
        {
            PActionItem actionPrefs = new PActionItem( new PLabel("Dial by URL"),
                    getString("hint/coreapp/idle/dialbyurl"),
                    m_dispatcher,
                    m_dispatcher.ACTION_DIAL_BY_URL) ;
            menuControl.setItem(PBottomButtonBar.B1, actionPrefs) ;
        }


        // New Call
        PActionItem actionNewCall = new PActionItem( new PLabel("New Call"),
                getString("hint/coreapp/idle/newcall"),
                m_dispatcher,
                m_dispatcher.ACTION_NEWCALL) ;
        menuControl.setItem(PBottomButtonBar.B2, actionNewCall) ;

    }


    /**
     * Initialize the menus that are displayed when a user hits the pingtel
     * key and selects the menu tab.
     */
    protected void initializeMenus()
    {
        PMenuComponent leftMenu = getLeftMenuComponent() ;
        PMenuComponent rightMenu = getRightMenuComponent() ;


        // Initialize left menu

        leftMenu.addItem(new PActionItem( new PLabel("Redial"),
                            getString("hint/coreapp/idle/redial"),
                            m_dispatcher,
                            m_dispatcher.ACTION_REDIAL)) ;

        if( Shell.getInstance().isSpeedDialEnabled() )
        {
            leftMenu.addItem(new PActionItem( new PLabel("Speed dial"),
                                        getString("hint/coreapp/idle/speeddial"),
                                        m_dispatcher,
                                        m_dispatcher.ACTION_SPEED_DIAL)) ;
        }

        if( Shell.getInstance().isCallLogEnabled() )
        {
            leftMenu.addItem(new PActionItem( new PLabel("Call Log"),
                                        getString("hint/coreapp/idle/calllog"),
                                        m_dispatcher,
                                        m_dispatcher.ACTION_CALL_LOG)) ;
        }

//        leftMenu.addItem(new PActionItem( new PLabel("Phonebook"),
//                                    getString("hint/coreapp/idle/phonebook"),
//                                    m_dispatcher,
//                                    m_dispatcher.ACTION_PHONEBOOK)) ;

        if( Shell.getInstance().isDialByUrlEnabled() )
        {
            leftMenu.addItem(new PActionItem( new PLabel("Dial by URL"),
                                        getString("hint/coreapp/idle/dialbyurl"),
                                        m_dispatcher,
                                        m_dispatcher.ACTION_DIAL_BY_URL)) ;
        }



        // Initialize right menu

        rightMenu.addItem(new PActionItem( new PLabel("About"),
                                    getString("hint/core/system/aboutform"),
                                    m_dispatcher,
                                    m_dispatcher.ACTION_ABOUT)) ;

        rightMenu.addItem(new PActionItem( new PLabel("Restart"),
                                    getString("hint/core/system/restart"),
                                    m_dispatcher,
                                    m_dispatcher.ACTION_RESTART)) ;


        rightMenu.addItem(new PActionItem( new PLabel("New Call"),
                                    getString("hint/coreapp/idle/newcall"),
                                    m_dispatcher,
                                    m_dispatcher.ACTION_NEWCALL)) ;




    }


    /**
     *
     */
    public void onDialByURL()
    {
        ApplicationManager appManager = ApplicationManager.getInstance() ;

        appManager.activateCoreApplication(ApplicationRegistry.DIAL_BY_URL_APP) ;
    }


    /**
     *
     */
    public void onRedial()
    {
        PAddress remoteAddress = CallLogManager.getInstance().getLastDialed();
        PAddress localAddress = CallLogManager.getInstance().getLocalAddressOfLastDialed();
        if (remoteAddress != null && localAddress != null) {
            Shell.getInstance().getDialingStrategy().dial(localAddress, remoteAddress) ;
        } else {
            getApplication().showMessageBox(MessageBox.TYPE_ERROR, "Cannot Redial", "No outgoing calls have been made on this phone since the last reset.") ;
            Shell.getInstance().getDialingStrategy().abort() ;
        }
    }


    /**
     *
     */
    public void onCallLog()
    {
        ApplicationManager appManager = ApplicationManager.getInstance() ;

        appManager.activateCoreApplication(ApplicationRegistry.CALL_LOG_APP) ;
    }


    /**
     *
     */
    public void onPhonebook()
    {
//        ApplicationManager appManager = ApplicationManager.getInstance() ;

//        appManager.activateCoreApplication(ApplicationRegistry.PHONEBOOK_APP) ;
        getApplication().showMessageBox("Phonebook", "I'm sorry, but this feature is still under construction.") ;
    }


    /**
     *
     */
    public void onPreferences()
    {
        getApplication().showMessageBox("Preferences", "I'm sorry, but this feature is still under construction.") ;
    }


    /**
     *
     */
    public void onSpeedDial()
    {
        ApplicationManager appManager = ApplicationManager.getInstance() ;

        appManager.activateCoreApplication(ApplicationRegistry.SPEED_DIAL_APP) ;
    }


    /**
     * This method is invoked as a response to the "Restart" menu item.  This
     * will force a reboot of the phone.  In theory, it should probably try
     * to shut the system down gracefully and then invoke a reboot.
     * <p>
     * A confirmation message warns the end user before restarting the phone.
     */
    public void onRestart()
    {
        MessageBox messageBox = new MessageBox(getApplication(), MessageBox.TYPE_WARNING) ;
        messageBox.setMessage(getString("lblWarningRestart")) ;

        if (messageBox.showModal() == MessageBox.OK) {
            PingerInfo.getInstance().reboot() ;
        }
    }


    /**
     * This method is invoked as a reponse to the "New Call" menu itmes.  If
     * pressed a new call is created using whatever audio device is currently
     * in use.  (hand set if on hook; otherwise speaker?)
     */
    public void onNewCall()
    {
        // Ask the shell to take care of it.
        ((CoreApp) getApplication()).doCreateOutgoingCall(PAudioDevice.ID_NONE) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

/**
 * populates an internal vector with cycle patterns. Cycle patterns for the
 * title bar is defined by the parameter PHONESET_HOME_SCREEN_TITLE.
 * Cycling is specified in a templates as a '\n' character.
 *  Users can specify as many cycling levels as they like.
 *  Cycling length (secs) is controlled by the parameter
 *  PHONESET_HOME_SCREEN_CYCLE_SEC.
 *  The following substitution strings are supported:
 *    {displayname}  -> Display Name of line identity
 *                       unquoted
 *    {sipurl}       -> {username}@{hostname}
 *
 *    {username}     -> username of the line identity
 *    {hostname}     -> hostname of the line identity
 *  And all the substitution characters defined in java.text.SimpleDateFormat
 *  are supported to format date and time. The list of the suported characters for
 *  date and time can be found
 *  at http://java.sun.com/products/jdk/1.2/docs/api/java/text/SimpleDateFormat.html .
 *  An example of a pattern is:
 *     Hotel Narayani \n {MMM dd yy  mm} after {hh} \n {displayname} \n Swiming Lessons at 10pm
 *  In the above example, there are four values that get cycled in the title bar.
 *  First one is just a name, second one has the date pattern, third one uses
 *  the substitition strings {displayname} and fourth is a static one.
 *  An example of how the title would say for the above pattern is:
 *
 *  Hotel Narayani
 *  Nov 21 02 15 after 10
 *  Pradeep Paudyal
 *  Swimming Lessons at 10pm
*/

protected void populateCyclePatternsForTitleBar()
{
    m_vCyclePatterns = new Vector();
    String strHomeScreenTitlebar = PingerConfig.getInstance()
        .getValue(PingerConfig.PHONESET_HOME_SCREEN_TITLE) ;
    //I need to parse the string and then decide to display it in cycles.
    //There may be more than 2 items in the list that need to be cycled.
    int iIndex = -1;

    //there is something weird about reading a line with "\n" in it by using
    //readLine method of BufferedReader. It somehow escapes it when it returns
    //it to you.
    String strDelimiter = "\\n";
    if( strHomeScreenTitlebar == null )
    {
        strHomeScreenTitlebar = SystemDefaults.TITLEBAR_PATTERN;
        //if its from the system, then the delimiter is the usual "\n"
        strDelimiter = "\n";
    }

    if( strHomeScreenTitlebar != null )
    {
        //continue to support old values
        if( strHomeScreenTitlebar.equalsIgnoreCase("TIME") )
        {
            strHomeScreenTitlebar = "{h:mm a MM/dd/yy}";
        }
        else if( strHomeScreenTitlebar.equalsIgnoreCase("EXTENSION") )
        {
            strHomeScreenTitlebar = "{displayname}<{sipurl}";
        }
        else if( strHomeScreenTitlebar.equalsIgnoreCase("CYCLE") )
        {
            strHomeScreenTitlebar = "{h:mm a MM/dd/yy} \n {displayname}<{sipurl}>";
        }
        while( (iIndex=strHomeScreenTitlebar.indexOf(strDelimiter)) != -1 )
        {
            String strToken = strHomeScreenTitlebar.substring(0, iIndex );
            if( strToken != null )
                m_vCyclePatterns.addElement( strToken );

            if( strHomeScreenTitlebar.length() >= iIndex+2 )
            {
                strHomeScreenTitlebar = strHomeScreenTitlebar.substring( iIndex+2 );
            }
            else
            {
                break;
            }
        }
        m_vCyclePatterns.addElement( strHomeScreenTitlebar );
        m_iNumberOfCycles = m_vCyclePatterns.size();
    }
}

/**
 * populates hashtable for the substititon strings that are supported.
 * If we add support for more substitution strings in the future, we just
 * need to change this method.
 */
protected void refreshHashtableForCyclingSubstitutions()
{
    String strIdentity = ((CoreApp) getApplication()).getDefaultUser();
    if(  (m_strIdentity == null)             ||
        !(m_strIdentity.equals(strIdentity)) ||
          m_hashCyclingSubstitutions.isEmpty()     )
    {
        m_strIdentity = strIdentity;
        SipParser sipParser = new SipParser( strIdentity );
        m_hashCyclingSubstitutions.put
            ("displayname", sipParser.getDisplayName());
        m_hashCyclingSubstitutions.put("username", sipParser.getUser());
        m_hashCyclingSubstitutions.put("hostname", sipParser.getHost());

        //sip url string. just sip:username@hostname
        SipParser builder = new SipParser() ;
        builder.setUser(sipParser.getUser()) ;
        builder.setHost(sipParser.getHost()) ;
        String strSipUrl = builder.render() ;
        strSipUrl.trim() ;
        ///
        m_hashCyclingSubstitutions.put("sipurl",  strSipUrl);

    }
}

/**
 * Initializes the settings for the title of this form.
 * Populates patterns, populates hashtable with subsitution strings and values,
 * and adds a timer if the display in the title is to be cycled.
 */
protected void initializeTitleSettings()
{
    populateCyclePatternsForTitleBar();
    refreshHashtableForCyclingSubstitutions();

    if( m_iNumberOfCycles <= 1 )
    {
        m_bCycle = false;
    }
    if( m_bCycle )
    {
        // Determine how long to wait if CYCLEing
        String strCyclePeriod = PingerConfig.getInstance()
            .getValue(PingerConfig.PHONESET_HOME_SCREEN_CYCLE_SEC) ;
        if ((strCyclePeriod != null) && (strCyclePeriod.length() > 0))
        {
            try
            {
                m_iCycleLength = Integer.parseInt(strCyclePeriod) ;
            }
            catch (NumberFormatException nfe)
            {
                SysLog.log(nfe);
            }
        }
        if( !m_bCycleTimerAdded )
        {
            Timer.getInstance().addTimer(m_iCycleLength * 1000, m_dispatcher, null, true) ;
            m_bCycleTimerAdded = true;
        }
    }
    refreshTitlebarCycle();
}


/**
 * This refreshes the titlebar with new information if there are more
 * than one cycling level. It looks for the substitution string inside
 * curly brace and substitutes values for it.
 */
protected void refreshTitlebarCycle()
{
    refreshHashtableForCyclingSubstitutions();
    //System.out.println("******************** hash is " + m_hashCyclingSubstitutions );
    String strPattern = (String) (m_vCyclePatterns.elementAt(m_iCycleOrder));
    //now find the keys enclosed with open curly brace and
    //closed curly brace and subsitute their values.
    //first substitute the ones with Pingtel keys, if not then
    //it should be a date key, if not throw an error.
    StringBuffer strTranslationBuf = new StringBuffer();
    StringTokenizer tokenizer = new StringTokenizer(strPattern, "{");
    if( !tokenizer.hasMoreTokens() )
        strTranslationBuf.append( strPattern );
    boolean bTimeAutoUpdate = false;

    // Clear any previously set timers
    if (m_dateTimeUpdater != null) {
         Timer.getInstance().removeTimers(m_dateTimeUpdater) ;
        m_dateTimeUpdater = null ;
    }

    while( tokenizer.hasMoreTokens() )
    {
        String strToken = tokenizer.nextToken();

        int iIndexOfClosingBrace = strToken.indexOf('}');
        if( iIndexOfClosingBrace == -1 )
        {
            //TO DO: This doesn't handle the error case when there
            //is only the open curly brace but no closing brace.
            //I guess I can take the count of open and close braces and
            //throw an error if they are not equal.
            strTranslationBuf.append( strToken );
        }
        else
        {
            String strPatternKey = strToken.substring(0, iIndexOfClosingBrace);
            String lowerCasePatternKey = strPatternKey.toLowerCase();
            if( m_hashCyclingSubstitutions.containsKey(lowerCasePatternKey) )
            {
                strTranslationBuf.append
                    ( m_hashCyclingSubstitutions.get(lowerCasePatternKey));
            }

            // Stick in system property (e.g. {$os.name})
            else if (strPatternKey.length() > 1 && strPatternKey.charAt(0) == '$')
            {
                 strTranslationBuf.append
                    ( System.getProperty(strPatternKey.substring(1), ""));
            }else
            {
                //it should be a date, if not throw an error.
                try
                {
                    SimpleDateFormat dateFormat = new SimpleDateFormat(strPatternKey);
                    //if the patterns contains time in minute and if the cycle interval
                    //is more than a minute, we need to do autoUpdate of the time every minute.
                    if( (strPatternKey.indexOf('m') != -1 ) && (m_iCycleLength > 60 ))
                    {
                        bTimeAutoUpdate = true;
                    }
                    strTranslationBuf.append
                        (dateFormat.format(Calendar.getInstance().getTime()));
                }catch( Exception e )
                {
                    //throw error once.
                    if( !m_bTitlebarDisplayErrorReported )
                    {
                        SysLog.log("Invalid pattern " + strPattern +
                             " defined for displaying information in the title bar.");
                        m_bTitlebarDisplayErrorReported = true;
                    }

                }
            }
            if( strToken.length() >= iIndexOfClosingBrace+1 )
                strTranslationBuf.append( strToken.substring(iIndexOfClosingBrace+1) );
        }

    }
    setTitle( strTranslationBuf.toString() );
    //if autoUpdate is true, we need to set a new timer to refresh after 60 sec
    if( bTimeAutoUpdate )
    {
        m_dateTimeUpdater = new icDateTimeUpdateCallback() ;

        // Figure out when the minute will change, and set the timer for 16 ms after it.
        int iFireNext = (int) (60000 - (Calendar.getInstance().getTime().getTime() % 60000) + 16) ;

        // We will set a one time timer and then the
        // icDateTimeUpdateCallback is smart enough to put us in auto
        // rearm mode.
        Timer.getInstance().resetTimer(iFireNext, m_dateTimeUpdater, null,false) ;
    }
}




protected void cleanupTitlebar()
{
    Timer.getInstance().removeTimers(m_dispatcher) ;
    m_bCycleTimerAdded = false;
}

//////////////////////////////////////////////////////////////////////////////
// Inner classes
////

    /**
    * This class waits for time events tell it that time has changed.
    */
    private class icDateTimeUpdateCallback implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            refreshTitlebarCycle();
        }
    }


    /**
     *
     */
    private class icEventDispatcher implements PActionListener
    {
        public final String ACTION_ABOUT		= "action_about" ;
        public final String ACTION_DIAL_BY_URL	= "action_dial_by_url" ;
        public final String ACTION_OUTSIDE_LINE = "action_outside_line" ;
        public final String ACTION_REDIAL		= "action_redial" ;
        public final String ACTION_CALL_LOG	    = "action_call_log" ;
        public final String ACTION_PHONEBOOK	= "action_phonebook" ;
        public final String ACTION_PREFERENCES	= "action_preferences" ;
        public final String ACTION_SPEED_DIAL	= "action_speed_dial" ;
        public final String ACTION_RESTART	    = "action_restart" ;
        public final String ACTION_NEWCALL      = "action_newcall" ;

        /**
         *
         */
        public void actionEvent(PActionEvent event)
        {

            if (event.getActionCommand().equals(ACTION_ABOUT))
            {
                SystemAboutBox box = new SystemAboutBox(getApplication());
                box.onAbout();
            }
            else if (event.getActionCommand().equals(ACTION_DIAL_BY_URL))
            {
                onDialByURL() ;
            }
            else if (event.getActionCommand().equals(ACTION_REDIAL))
            {
                onRedial() ;
            }
            else if (event.getActionCommand().equals(ACTION_CALL_LOG))
            {
                onCallLog() ;
            }
            else if (event.getActionCommand().equals(ACTION_PHONEBOOK))
            {
                onPhonebook() ;
            }
            else if (event.getActionCommand().equals(ACTION_PREFERENCES))
            {
                onPreferences() ;
            }
            else if (event.getActionCommand().equals(ACTION_SPEED_DIAL))
            {
                onSpeedDial() ;
            }
            else if (event.getActionCommand().equals(ACTION_RESTART))
            {
                onRestart() ;
            }
            else if (event.getActionCommand().equals(ACTION_NEWCALL))
            {
                onNewCall() ;
            }
            else if (event.getActionCommand().equals(Timer.ACTION_TIMER_FIRED))
            {
                refreshTitlebarCycle() ;
                m_iCycleOrder++;
                if( m_iNumberOfCycles == m_iCycleOrder)
                    m_iCycleOrder = 0;
            }
        }
    }


    private class icButtonListener implements PButtonListener
    {
        /**
         * The specified button has been press downwards
         */
        public void buttonDown(PButtonEvent event)
        {
            PCallManager callManager = Shell.getCallManager() ;
            PCall call = null ;
            switch (event.getButtonID()) {

                case PButtonEvent.BID_0:
                case PButtonEvent.BID_1:
                case PButtonEvent.BID_2:
                case PButtonEvent.BID_3:
                case PButtonEvent.BID_4:
                case PButtonEvent.BID_5:
                case PButtonEvent.BID_6:
                case PButtonEvent.BID_7:
                case PButtonEvent.BID_8:
                case PButtonEvent.BID_9:
                case PButtonEvent.BID_STAR:
                    if (m_strDialierHandoff == null) {
                        m_callHandoff = ((CoreApp) getApplication()).startDialer() ;
                        if (m_callHandoff != null) {
                            m_strDialierHandoff =  "" + (char) event.getButtonID() ;
                            m_generator.beginDTMFTone(event.getButtonID(), m_callHandoff) ;
                        }
                    } else {
                        if (m_callHandoff != null) {
                            m_strDialierHandoff += (char) event.getButtonID() ;
                            m_generator.beginDTMFTone(event.getButtonID(), m_callHandoff) ;
                        }
                    }
                    break ;
                }
        }


        /**
         * The specified button has been released
         */
        public void buttonUp(PButtonEvent event)
        {
            PCallManager callManager = Shell.getCallManager() ;

            switch (event.getButtonID()) {
                case PButtonEvent.BID_0:
                case PButtonEvent.BID_1:
                case PButtonEvent.BID_2:
                case PButtonEvent.BID_3:
                case PButtonEvent.BID_4:
                case PButtonEvent.BID_5:
                case PButtonEvent.BID_6:
                case PButtonEvent.BID_7:
                case PButtonEvent.BID_8:
                case PButtonEvent.BID_9:
                case PButtonEvent.BID_STAR:
                    m_generator.endTone(callManager.getInFocusCall()) ;
                    event.consume() ;
                    break ;
            }
        }


        /**
         * The specified button is being held down
         */
        public void buttonRepeat(PButtonEvent event)
        {
        }
    }
}

