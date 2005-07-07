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

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.featureindicator.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.ShellApp ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.calllog.* ;
import org.sipfoundry.sipxphone.sys.app.shell.*;
import org.sipfoundry.sipxphone.sys.app.core.RestartConfirmationForm ;

import org.sipfoundry.sipxphone.app.preferences.CallHandlingManager ;
import org.sipfoundry.sipxphone.app.preferences.DoNotDisturbFeatureIndicator ;
import org.sipfoundry.sipxphone.app.preferences.CallForwardingFeatureIndicator ;
import org.sipfoundry.sipxphone.sys.appclassloader.ApplicationManager ;
import org.sipfoundry.sipxphone.awt.PCommandBar ;
import org.sipfoundry.sipxphone.awt.PBottomButtonBar ;
import org.sipfoundry.sipxphone.awt.PMenuComponent ;
import org.sipfoundry.sipxphone.sys.app.core.IdleForm ;

import java.util.Vector;
import java.util.Enumeration;
import java.io.InputStream;
import java.io.IOException;
import java.net.URL;
import java.net.MalformedURLException;

import org.sipfoundry.stapi.* ;
import org.sipfoundry.telephony.* ;


/**
 * The Shell is a collection point for most of the system functions and
 * objects.
 * <p>
 * Developers can obtain a reference to the singleton Shell object by calling
 * the Shell's <i>getInstance</i> method. For example:
 * <PRE>
 * PCallManager callManager = Shell.getInstance().getCallManager();
 * </PRE>
 *
 * @see #getInstance
 * @author Robert J. Andreasen, Jr.
 */
public class Shell
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** Singleton reference to the one and only shell instance. */
    private static Shell m_shellReference = null ;
    private static PCallManager m_callManager = null ;
    private static PMediaManager m_mediaManager = null ;
    private static CallLog m_callLog = null ;
    private static HookManager m_hookManager = null ;
    private static FeatureIndicatorManager m_indicatorManager = null ;
    private static XpressaSettings m_xpressaSettings = null ;
    private static CallHandlingManager m_callHandlingManager = null;

    private String m_strDialerFunction = null ;
    private PActionListener m_listenerDialerHandler = null ;
    private int m_iDialerHandlerRefCount = 0 ;
    private DialingStrategy m_defaultDialingStrategy ;
    private DialingStrategy m_currentDialingStrategy ;
    private PCall m_callStrategy ;
    private MessageWaitingIndicatorManager m_waitingIndicatorManager ;

    protected URL m_musicOnHoldURL;

    /** is call log enabled for this system? */
    private boolean m_bCalllogEnabled = true;
    /** is speed dial enabled for this system? */
    private boolean m_bSpeeddialEnabled = true;
    /** is dial by url enabled for this system? */
    private boolean m_bDialByUrlEnabled = true;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default Constructor; protected to help enforce a singleton design
     * pattern.
     */
    protected Shell()
    {
        initDefaultDialingStrategy() ;

        m_waitingIndicatorManager = new MessageWaitingIndicatorManager() ;

        //find if calllog is enabled.
        if( !ApplicationRegistry.getInstance().isApplicationRegistered
            ( ApplicationRegistry.CALL_LOG_APP))
        {
            m_bCalllogEnabled = false;
        }
        //find if speed dial is enabled.
        if( !ApplicationRegistry.getInstance().isApplicationRegistered
            ( ApplicationRegistry.SPEED_DIAL_APP))
        {
            m_bSpeeddialEnabled = false;
        }
        //find if dial by url is enabled.
        if( !ApplicationRegistry.getInstance().isApplicationRegistered
            ( ApplicationRegistry.DIAL_BY_URL_APP))
        {
            m_bDialByUrlEnabled = false;
        }

        initMusicOnHoldURL();
    }


    /**
     * Get a reference to the Shell singleton object.
     *
     * @exception SecurityException Thrown if the user/application does not have
     *            permission to perform the operation.
     *
     * @return The Shell reference.
     */
    public static Shell getInstance()
        throws SecurityException
    {
        // Create if needed
        if (m_shellReference == null) {
            m_shellReference = (Shell) new Shell() ;
        }

        return m_shellReference ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * @deprecated do not expose!
     */
    public URL getMusicOnHoldURL()
    {
        return m_musicOnHoldURL;
    }

    /**
     *Is Call Log Enabled for this system?
     * @deprecated do not expose!
     */
    public boolean isCallLogEnabled()
    {
        return m_bCalllogEnabled;
    }

    /**
     * Is  Speed Dial Enabled for this system?
     * @deprecated do not expose!
     */
    public boolean isSpeedDialEnabled()
    {
        return m_bSpeeddialEnabled;
    }

    /**
     * Is Dial By URL enabled for this system?
     * @deprecated do not expose!
     */
    public boolean isDialByUrlEnabled()
    {
        return m_bDialByUrlEnabled;
    }

    /**
     * @deprecated do not expose!
     */
    public void setMessageIndicator(int iMode, String strIdentifier)
    {
        m_waitingIndicatorManager.setState(iMode, strIdentifier) ;
    }


    /**
     * Show a message box given a type, title, text, and application.
     *
     * @param iType The type of message box.  See the type constants within
     *        {@link MessageBox} for a detailed description.
     * @param strTitle The title of the message box.  If this parameter is null, a default will be
     *        provided.
     * @param strText The text inside the message box.  This parameter should not be null.
     * @param app The reference to the application object requesting the message
     *        box.
     *
     * @exception SecurityException Thrown if the user/application does not have
     *            permission to perform the operation.
     *
     * @see org.sipfoundry.sipxphone.awt.form.MessageBox
     * @see org.sipfoundry.sipxphone.Application
     */
    public int showMessageBox(int iType, String strTitle, String strText, Application app)
        throws SecurityException
    {
        if (app == null)
            app = ShellApp.getInstance() ;

        MessageBox form = new MessageBox(app, iType) ;

        form.setTitle(strTitle) ;
        form.setMessage(strText) ;

        return form.showModal() ;
    }


    /**
     * @deprecated do not expose
     */
    public void showUnhandledException(Throwable t, boolean bBlock)
    {
        SysLog.log(t);

        MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;

        messageBox.setTitle(messageBox.getString("lblErrorTitleUnhandledException")) ;
        messageBox.setMessage(messageBox.getString("lblErrorTextUnhandledException") + t.toString()) ;

        if (bBlock)
            messageBox.showModal() ;
        else
            messageBox.showModeless() ;
    }


    /**
     * Display a flyover for the specified duration.  A flyover text frame is
     * filled with user-supplied data and hovers over the lower portion of the
     * screen. A flyover disappears either after the desired duration or when a button
     * is pressed.
     *
     * @param strText The text that should display in the flyover.  The text
     *        will be automatically wrapped on word boundaries.
     *        If you wish to force the text to a new line, insert a "\n" into
     *        the text.
     * @param iDuration The duration that the flyover should be displayed, in milliseconds.
     *
     * @exception SecurityException Thrown if the user/application does not have
     *            permission to perform the operation.
     *
     * @see #clearFlyOver
     *
     * @deprecated do not expose.
     */
    public void displayFlyOver(String strText, int iDuration)
        throws SecurityException
    {
        displayFlyOver( strText, iDuration, SystemDefaults.FONTID_DEFAULT_SMALL );
    }

     /**
     * Display a flyover for the specified duration, with the specified font.
     * A flyover text frame is
     * filled with user-supplied data and hovers over the lower portion of the
     * screen. A flyover disappears either after the desired duration or when a button
     * is pressed.
     *
     * @param strText The text that should display in the flyover.  The text
     *        will be automatically wrapped on word boundaries.
     *        If you wish to force the text to a new line, insert a "\n" into
     *        the text.
     * @param iDuration The duration that the flyover should be displayed, in milliseconds.
     * @param iFont font to display the test in the flyover
     *
     * @exception SecurityException Thrown if the user/application does not have
     *            permission to perform the operation.
     *
     * @see #clearFlyOver
     *
     * @deprecated do not expose.
     */
    public void displayFlyOver(String strText, int iDuration, int iFont)
        throws SecurityException
    {
        Vector vDurationAndFont = new Vector();
        vDurationAndFont.addElement(new Integer(iDuration));
        vDurationAndFont.addElement(new Integer(iFont));
        ShellEvent event = new ShellEvent(ShellEvent.SE_SHOWFLYOVER, this, strText, vDurationAndFont) ;
        ShellApp.getInstance().postEvent(event) ;
    }



    /**
     * This method clears and hides any flyovers that are currently being displayed.
     *
     * @exception SecurityException Thrown if the user/application does not have
     *            permission to clear the flyover.
     *
     * @exception SecurityException Thrown if the user/application does not have
     *            permission to perform the operation.
     *
     * @see #displayFlyOver
     *
     * @deprecated do not expose.
     */
    public void clearFlyOver()
        throws SecurityException
    {
        ShellEvent event = new ShellEvent(ShellEvent.SE_CLEARFLYOVER, this, null, null) ;
        ShellApp.getInstance().postEvent(event) ;
    }


    /**
     * Display a hint.  A hint text frame is filled with user-supplied data
     * and hovers over the lower portion of the screen until a button up is
     * detected.  A hint displays when a button is held down for a
     * period of time.
     *
     * @param strText The text should be display in the hint window.  This
     *                text should be formed as "title|hint text" (without
     *                the quotes.  The title is optional and a pipe is used
     *                as a delimiter.  The hint text will be wrapped
     *                automatically, however you can also insert \n's.
     *
     * @exception SecurityException Thrown if the user/application does not have
     *            permission to display a hint.
     *
     * @see #clearHint
     *
     * @deprecated do not expose.
     */
    public void displayHint(String strText)
        throws SecurityException
    {
        if (strText != null) {
            String strHintTitle = null ;
            String strHintText = null ;

            int iDelimiterPosition = strText.indexOf('|') ;
            if (iDelimiterPosition != -1) {
                strHintTitle = strText.substring(0, iDelimiterPosition) ;
                if (strText.length() > (iDelimiterPosition+1))
                    strHintText = strText.substring(iDelimiterPosition+1) ;
            } else {
                strHintText = strText ;
            }

            ShellEvent event = new ShellEvent(ShellEvent.SE_SHOWHINT, this, strHintText, strHintTitle) ;
            ShellApp.getInstance().postEvent(event) ;
        } else
            clearHint() ;
    }


    /**
     * Display a hint.  A hint text frame is filled with user-supplied data
     * and hovers over the lower portion of the screen until a button up is
     * detected.  A hint displays when a button is held down for a
     * period of time.
     *
     * @param strText The text that should display in the hint.  The text
     *        will be automatically wrapped on word boundaries.
     *        If you wish to force the text to a new line, insert a "\n" into
     *        the text.
     * @param strTitle The title text that will be displayed for the hint.
     *
     * @exception SecurityException Thrown if the user/application does not have
     *            permission to display a hint.
     *
     * @see #clearHint
     *
     * @deprecated do not expose.
     */
    public void displayHint(String strText, String strTitle)
        throws SecurityException
    {
        ShellEvent event = new ShellEvent(ShellEvent.SE_SHOWHINT, this, strText, strTitle) ;
        ShellApp.getInstance().postEvent(event) ;
    }



    /**
     * Clear and hide any hints that are currently being displayed on the screen.
     *
     * @exception SecurityException Thrown if the user/application does not have
     *            permission to clear the hint.
     *
     * @see #displayHint
     *
     * @deprecated do not expose.
     */
    public void clearHint()
        throws SecurityException
    {
        ShellEvent event = new ShellEvent(ShellEvent.SE_CLEARHINT, this, null, null) ;
        ShellApp.getInstance().postEvent(event) ;
    }


    /**
     * Get a reference to the Call Manager.  The Call Manager is used
     * to view and interact with calls on this phone.
     *
     * @return {@link PCallManager}: the Call Manager
     *
     * @exception SecurityException Thrown if the user/application does not have
     *            permission to perform the operation.
     *
     * @see org.sipfoundry.stapi.PCallManager
     */
    public static PCallManager getCallManager()
        throws SecurityException
    {
        if (m_callManager == null)
            m_callManager = ShellApp.getInstance().getCallManager() ;

        return m_callManager ;
    }


    /**
     * Get a reference to the xpressa settings object.  This object is used to
     * view various phone and network settings.
     *
     * @return The XpressaSettings object
     *
     * @exception SecurityException Thrown if the user/application does not have
     *            permission to perform the operation.
     *
     * @see org.sipfoundry.sipxphone.sys.XpressaSettings
     */
    public static XpressaSettings getXpressaSettings()
        throws SecurityException
    {
        if (m_xpressaSettings == null)
            m_xpressaSettings = new XpressaSettings() ;

        return m_xpressaSettings ;
    }



    /**
     * Get a reference to the Media Manager.  The Media Manager is used to
     * switch to different audio devices and set volumes.
     *
     * @return {@link PMediaManager}: the Media Manager.
     *
     * @exception SecurityException Thrown if the user/application does not have
     *            permission to perform the operation.
     *
     * @see org.sipfoundry.stapi.PMediaManager
     */
    public static PMediaManager getMediaManager()
        throws SecurityException
    {
        if (m_mediaManager == null)
            m_mediaManager = ShellApp.getInstance().getMediaManager() ;

        return m_mediaManager ;

    }


    /**
     * @deprecated <B>Reserved for future use.</B>
     */
    public static CallLog getCallLog()
        throws SecurityException
    {
        if (m_callLog == null)
            m_callLog = new CallLog() ;

        return m_callLog ;
    }


    /**
     * Activate the specified application.  This will launch the application
     * if not running, otherwise, bring it into focus.
     *
     * @deprecated <B>Reserved for future use.</B>
     *
     * @return true if successful, otherwise false
     */
    public boolean activateApplication(String strClassNameOrPath, String[] args)
    {
        ApplicationManager mgrApp = ApplicationManager.getInstance() ;

        return mgrApp.activateApplication(strClassNameOrPath, args) ;
    }


    /**
     * Activate the specified core application.  This will launch the
     * application if not running, otherwise, bring it into focus.
     *
     * @deprecated <B>Reserved for future use.</B>
     *
     * @return true if successful, otherwise false
     */
    public boolean activateCoreApplication(int iAppType, String[] args)
    {
        ApplicationManager mgrApp = ApplicationManager.getInstance() ;

        return mgrApp.activateCoreApplication(iAppType, args) ;
    }



    /**
     * @deprecated do not expose
     */
    public static FeatureIndicatorManager getFeatureIndicatorManager()
    {
        if (m_indicatorManager == null)
            m_indicatorManager = new FeatureIndicatorManager() ;

        return m_indicatorManager ;
    }


    /**
     * Get a reference to the Hook Manager.  The Hook Manager allows you to
     * override default system behaviors.
     *
     * @return {@link HookManager}: the Hook Manager.
     *
     * @exception SecurityException Thrown if the user/application does not have
     *            permission to perform the operation.
     *
     * @see org.sipfoundry.sipxphone.sys.HookManager
     */
    public static HookManager getHookManager()
        throws SecurityException
    {
        if (m_hookManager == null)
            m_hookManager = new HookManager() ;

        return m_hookManager ;
    }


    /**
     * Get a reference to the Call Handling Manager.  The Call Handling Manager
     * allows you to change call handling behaviors.
     *
     * @return {@link CallHandlingManager}: the Call Handling Manager.
     *
     * @see org.sipfoundry.sipxphone.app.preferences.CallHandlingManager
     *
     * @deprecated DO NOT EXPOSE
     */
    public static CallHandlingManager getCallHandlingManager()
    {
        if (m_callHandlingManager == null)
            m_callHandlingManager = new CallHandlingManager() ;

        return m_callHandlingManager ;
    }


    /**
     * This method displays a non-cancelable status frame and
     * the objSource of the ShellEvent will be Shell itself.
     * @param strText text to be displayed in the status frame.
     * @deprecated <B>Reserved for future use.</B>
     */
    public void displayStatus(String strText)
        throws SecurityException
    {
        displayStatus( strText, null );
    }

    /**
     * This method displays a non-cancelable status frame and
     * the objSource of the ShellEvent will be Shell itself.
     * @param strText text to be displayed in the status frame.
     * @param strParam extra parameter that is to be passed to ShellEvent.
     * @deprecated <B>Reserved for future use.</B>
     */
    public void displayStatus(String strText, String strParam)
        throws SecurityException
    {
        ShellEvent event =
            new ShellEvent(ShellEvent.SE_SHOWSTATUS, this, strText, strParam) ;
        ShellApp.getInstance().postEvent(event) ;
    }



    /**
     * displays a canceleable status window.
     * Those who want to take action when status gets displayed, closed or
     * "cancel"  is pressed , need to pass an instance of  PStatusListener.
     * NOTE: NOTE: NOTE:
     * Not more than one source should call displayCancelableStatus
     * at any time as it is not supported yet. And if you call this method,
     * make sure you call clearStatus() so that other sources can use it too.
     *
     * @deprecated <B>NOT EXPOSED</B>
     */
    public void displayCancelableStatus
        (String strText, Object objSource, PStatusListener listener) throws
        SecurityException
    {
        if( objSource == null ){
            throw new SecurityException("objSource cannot be null");
        }
        ShellEvent event =
            new ShellEvent(ShellEvent.SE_SHOWSTATUS, objSource,
                           strText, listener) ;
        ShellApp.getInstance().postEvent(event) ;
    }



    /**
     * clears the status window in iTimeout milliseconds.
     * @deprecated <B>Reserved for future use.</B>
     */
    public void clearStatus(int iTimeout)
        throws SecurityException
    {
        ShellEvent event =
            new ShellEvent(ShellEvent.SE_CLEARSTATUS,
                           this,
                           new Integer(iTimeout),
                           null) ;
        ShellApp.getInstance().postEvent(event) ;
    }

    /**
     * clears the status window with a timeout of 0 milliseconds
     *  @deprecated do not expose, yet.
     */
    public void clearStatus()
    {
        clearStatus(0);
    }


    /**
     * Sets the dialing strategy use by core applications.  The system and
     * applications can change the behavior of 'dial' by setting one of
     * these dial strategies.  A simple usage example is transfer, where
     * an application doens't issue a connect(), but rather, a transfer().
     *
     * @deprecated do not expose, yet.
     */
    public void setDialingStrategy(DialingStrategy strategy)
    {
        setDialingStrategy(strategy, null) ;
    }


    /**
     * @deprecated do not expose, yet.
     */
    public void setDialingStrategy(DialingStrategy strategy, PCall call)
    {
        if (strategy == null)
            m_currentDialingStrategy = m_defaultDialingStrategy ;
        else
            m_currentDialingStrategy = strategy ;

        m_callStrategy = call ;
    }


    /**
     * Get the default dialing strategy.  This is the strategy used when the
     * user is not dialing as part of an core application (such as conference
     * or transfer).  Potentially, a user application, could also implement
     * a dialing strategy.
     *
     * @deprecated do not expose, yet.
     */
    public DialingStrategy getDefaultDialingStrategy()
    {
        return m_defaultDialingStrategy ;
    }


    /**
     * @deprecated do not expose, yet.
     */
    public void addDialerFormListener(PFormListener listener)
    {
        ShellApp.getInstance().getCoreApp().addDialerFormListener(listener) ;
    }


    /**
     * HACK: Users may need to monitor the dialer form.  For now, this proxy
     * acts a temp solution, I'm not sure what the correct interface/method
     * should be.  The dialer is abstracted away from user applications with
     * the dialing stategy and dial() methods on this object.  Perhaps dial
     * should take some sort of dial listener or a form listener.
     *
     * @deprecated do not expose, yet.
     */
    public void removeDialerFormListener(PFormListener listener)
    {
        ShellApp.getInstance().getCoreApp().removeDialerFormListener(listener) ;
    }




    /**
     * HACK: Users may need to monitor the dialer form.  For now, this proxy
     * acts a temp solution, I'm not sure what the correct interface/method
     * should be.  The dialer is abstracted away from user applications with
     * the dialing stategy and dial() methods on this object.  Perhaps dial
     * should take some sort of dial listener or a form listener.
     *
     * @deprecated do not expose, yet.
     */
    public void dial()
    {
        ShellApp.getInstance().postEvent(new ShellEvent(ShellEvent.SE_DIAL, this, null, null)) ;
    }


    /**
     * Gets the dialing strategy specified by the system or an application.
     *
     * @deprecated do not expose, yet.
     */
    public DialingStrategy getDialingStrategy()
    {
        return m_currentDialingStrategy ;
    }


    /**
     * @deprecated do not expose, yet.
     */
    public PCall getDialingStrategyCall()
    {
        return m_callStrategy ;
    }


    /**
     * @deprecated do not expose, yet.
     */
    public PBottomButtonBar getHomeScreenButtonBar()
    {
        IdleForm form = ShellApp.getInstance().getCoreApp().getIdleForm() ;
        return form.getBottomButtonBar() ;
    }


    /**
     * @deprecated do not expose, yet.
     */
    public PCommandBar getHomeScreenCommandBar()
    {
        IdleForm form = ShellApp.getInstance().getCoreApp().getIdleForm() ;
        return form.getCommandBar() ;
    }


    /**
     * @deprecated do not expose, yet.
     */
    public PMenuComponent getHomeScreenLeftMenu()
    {
        IdleForm form = ShellApp.getInstance().getCoreApp().getIdleForm() ;
        return form.getLeftMenuComponent() ;
    }


    /**
     * @deprecated do not expose, yet.
     */
    public PMenuComponent getHomeScreenRightMenu()
    {
        IdleForm form = ShellApp.getInstance().getCoreApp().getIdleForm() ;
        return form.getRightMenuComponent() ;
    }



//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Initializes the music on hold.
     */
    protected  void initMusicOnHoldURL()
    {
        String strMusicOnHoldURL = PingerConfig.getInstance().getValue(
                PingerConfig.PHONESET_MUSIC_ON_HOLD_SOURCE) ;
        String strMusicOnHoldCondition = PingerConfig.getInstance().getValue(
                PingerConfig.PHONESET_MUSIC_ON_HOLD) ;

        m_musicOnHoldURL = null;

        if( strMusicOnHoldCondition == null || !strMusicOnHoldCondition.equalsIgnoreCase("enable"))
            strMusicOnHoldCondition = "disable";
        if(  strMusicOnHoldCondition.equalsIgnoreCase("enable")
           && !(TextUtils.isNullOrSpace( strMusicOnHoldURL))           )
        {
            try
            {
                m_musicOnHoldURL = new URL(strMusicOnHoldURL);
            }
            catch( MalformedURLException e)
            {
                e.printStackTrace();
            }
        }
    }


    protected void initDefaultDialingStrategy()
    {
        String strRingdownMethod = PingerConfig.getInstance().getValue(PingerConfig.PHONESET_RINGDOWN) ;
        String strRingdownAddress = PingerConfig.getInstance().getValue(PingerConfig.PHONESET_RINGDOWN_ADDRESS) ;

        m_defaultDialingStrategy = null ;
        if (    (strRingdownMethod != null) &&
                ((strRingdownMethod.equalsIgnoreCase("PROXY")) ||
                (strRingdownMethod.equalsIgnoreCase("IMMEDIATE"))) &&
                (strRingdownAddress != null) &&
                (strRingdownAddress.trim().length() > 0))
        {
            try
            {
                m_defaultDialingStrategy = new RingdownProxyDialingStrategy(strRingdownAddress.trim()) ;
            }
            catch (Exception e)
            {
                SysLog.log("Error while attempting to initialize proxy ringdown proxy address") ;
                SysLog.log(e) ;
            }
        }

        if (m_defaultDialingStrategy == null)
        {
            m_defaultDialingStrategy = new DefaultDialingStrategy() ;
        }

        setDialingStrategy(m_defaultDialingStrategy) ;
    }


    /**
     * @deprecated do not expose.
     *
     * WARNING: This is invoked from JNI, do not change the method signature!
     */
    protected static void onConfigurationChanged()
    {
        // Dump and reload data information
        System.out.println("onConfigurationChanged: Flushing configuration data...") ;
        PingerConfig config = PingerConfig.getInstance() ;
        config.reload() ;

        // Re-apply call handling settings
         System.out.println("onConfigurationChanged: Applying Call Handling Rules...") ;
         CallHandlingManager managerCallHandling = getCallHandlingManager() ;
        //managerCallHandling.commit() ;

        // Install / Uninstall DND feature indicator
        DoNotDisturbFeatureIndicator fiDND = DoNotDisturbFeatureIndicator.getInstance() ;
        if (fiDND.shouldInstall())
            fiDND.install() ;
        else
            fiDND.uninstall() ;

        CallForwardingFeatureIndicator fiCW = CallForwardingFeatureIndicator.getInstance() ;
        if (fiCW.shouldInstall())
            fiCW.install() ;
        else
            fiCW.uninstall() ;
    }


    /**
     * Load a resource and return it as an array of bytes.  This routine
     * is invoked from JNI, so do not change the method signature without
     * changing it in JNI.
     *
     * @deprecated do not expose
     */
    public static byte[] getXpressaResource(String strResource)
    {
        int iSize = 0 ;
        byte data[] = null ;

        Runtime.getRuntime().gc() ;
        Runtime.getRuntime().runFinalization() ;

        try
        {
            InputStream stream = ClassLoader.getSystemResourceAsStream("resources/" + strResource) ;
            if (stream != null)
            {
                iSize = stream.available() ;
                data = new byte[iSize] ;
                int iRead = 0;
				while (iRead < iSize)
				{
                    iRead += (int)stream.read(data, iRead, iSize - iRead) ;
				}
            }
                else
                System.out.println("NOT FOUND!!") ;

        }
        catch (IOException ioe)
        {
            SysLog.log(ioe) ;
        }

        return data ;
    }

    /**
     * @deprecated this should not be used and will be removed shortly
     *
     * @param strCallID callID where the part needs to be added
     * Note: this method is used by the web UI
     */
    protected static void addParty(String strCallID, String strURL)
    {
        PCall call = Shell.getCallManager().getCallByCallID(strCallID);
        if((call != null) && (strURL != null)){
            try{
                call.connect(new PAddress(strURL));
            }catch( SecurityException se){
                se.printStackTrace();
            }catch( PCallResourceUnavailableException pcru){
                pcru.printStackTrace();
            }catch( PCallAddressException pcae){
                pcae.printStackTrace();
            }
        }
    }

    /**
     * @deprecated this should not be used and will be removed shortly
     *
     * @param strCallID callID where the party needs to be removed from
     * Note: this method is used by the web UI
     */
    protected static void removeParty(String strCallID, String strURL)
    {
        PCall call = Shell.getCallManager().getCallByCallID(strCallID);
        if((call != null) && (strURL != null)){
            try{
                call.disconnect(new PAddress(strURL));
            }catch( SecurityException se){
                se.printStackTrace();
            }catch( PCallStateException pcse){
                pcse.printStackTrace();
            }
        }
    }

    /**
     * @deprecated this should not be used and will be removed shortly
     *
     * Note: this method is used by the web UI
     */
    protected static void hold(String strCallID, boolean bEnable)
    {
        PCall call = Shell.getCallManager().getCallByCallID(strCallID);
        if(call != null){
            try{
                if( bEnable ){
                    call.placeOnHold();
                }else{
                    call.releaseFromHold();
                }
            }catch( PCallStateException pcse){
                pcse.printStackTrace();
            }
        }
    }


    /**
     * @deprecated this should not be used and will be removed shortly
     *
     * Note: this method is used by the web UI
     */
    protected static void answer(String strCallID)
    {
        PCall incomingCall = Shell.getCallManager().getCallByCallID(strCallID);
        if(incomingCall != null){
            PCall callInFocus = Shell.getCallManager().getInFocusCall() ;
            if (callInFocus != incomingCall) {
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
                    incomingCall.stopSound() ;
                } catch (Exception e) { }
                incomingCall.answer() ;
                incomingCall.releaseFromHold() ;
                Shell.getMediaManager().setDefaultAudioDevice() ;
            } catch (PSTAPIException e) {
                // ::TODO:: Handle this failure condition
                SysLog.log(e) ;
            }
        }
    }




    /**
     * @deprecated this should not be used and will be removed shortly
     *
     * Note: this method is used by the web UI
     */
    protected static String simpleDial(String strURL)
    {
        boolean bCreatedCall = false ;
        String strCallID = "" ;

        // If this looks like a phone number, then remove all the dashes, dots,
        // whitespace, extensions, etc.
        if (PhoneNumberScrubber.looksLikePhoneNumber(strURL))
            strURL = PhoneNumberScrubber.scrub(strURL) ;

        PCall call = getCallManager().getInFocusCall() ;
        if (call == null)
        {
            try
            {
                call = PCall.createCall() ;
                bCreatedCall = true ;
                Shell.getMediaManager().setDefaultAudioDevice() ;
            }
            catch (PCallException e)
            {
                SysLog.log(e) ;

                MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                messageBox.setMessage(messageBox.getString("lblErrorMaxConnections")) ;
                messageBox.showModeless() ;
            }
            catch (PMediaException e)
            {
                SysLog.log(e) ;
            }
        }


        if (call != null)
        {
            try
            {
                call.connect(PAddressFactory.getInstance().createAddress(strURL)) ;
            }
            catch (PCallResourceUnavailableException e)
            {
                try
                {
                    if (bCreatedCall)
                        call.disconnect() ;
                }
                catch (Exception ee)
                {
                    SysLog.log(ee) ;
                }

                MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                messageBox.setMessage(messageBox.getString("lblErrorMaxConnections")) ;
                messageBox.showModeless() ;
                Shell.getInstance().getDefaultDialingStrategy().abort() ;
            }
            catch (PCallAddressException cae)
            {
                try
                {
                    if (bCreatedCall)
                        call.disconnect() ;
                }
                catch (Exception ee)
                {
                    SysLog.log(ee) ;
                }

                MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                messageBox.setMessage(cae.getMessage()) ;
                messageBox.showModeless() ;
                Shell.getInstance().getDefaultDialingStrategy().abort() ;
            }
            catch (Exception e) {
                try
                {
                    if (bCreatedCall)
                        call.disconnect() ;
                }
                catch (Exception ee)
                {
                    SysLog.log(ee) ;
                }

                MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                messageBox.setMessage(messageBox.getString("lblErrorTextUnhandledException") + e.getMessage()) ;
                messageBox.showModeless() ;
                Shell.getInstance().getDefaultDialingStrategy().abort() ;
            }
            strCallID = call.toString() ;
        }

        return strCallID ;
    }



    /**
     * Asks the user if they would like to restart.  If the user does not
     * respond within a sensible time period, the phone will return true
     * for the user.
     *
     * WARNING: This method is called from JNI.  Do not change the method
     *          signature without updating JXAPI.cpp
     *
     * @deprecated do not expose
     */
    protected static boolean requestRestart(int iSeconds, String strReason)
    {
        Shell.getInstance().clearStatus();

        RestartConfirmationForm form = new RestartConfirmationForm(
                ShellApp.getInstance().getCoreApp(),
                iSeconds,
                strReason) ;

        return (form.showModal(true) == form.OK) ;
    }



    /**
     * Determines if the phone is 'busy'.  For today, the present of any
     * calls is considered busy.  This concept may expand to cover opened
     * user applications, etc.
     *
     * WARNING: This method is called from JNI.  Do not change the method
     *          signature without updating JXAPI.cpp
     *
     * @deprecated do not expose
     */
    protected static boolean isPhoneBusy()
    {
        PCallManager manager = getCallManager() ;
        PCall calls[] = manager.getCalls() ;

        return (calls.length > 0) ;
    }


    /**
     * @deprecated this should not be used and will be removed shortly
     */
    protected static void simpleTransfer(String strURL, String strCallID)
    {

        PCall call = getCallManager().getInFocusCall() ;
        if (call != null) {
            try
            {
                call.transfer(PAddressFactory.getInstance().createAddress(strURL)) ;
            }
            catch (Exception e)
            {

            }
        }
    }


    /**
     * @deprecated this should not be used and will be removed shortly
     */
    protected static void doGarbageCollect()
    {
        System.gc() ;
    }


    protected static void doDisplayStatus(String strStatus)
    {
        Shell shell = Shell.getInstance() ;
        if (shell != null)
            shell.displayStatus(strStatus) ;
    }


    protected static void doClearStatus()
    {
        Shell shell = Shell.getInstance() ;
        if (shell != null)
            shell.clearStatus(0) ;
    }

    /**
     * gets an instance of Coreapp and sets the phones state whether a file is being transfered
     *
     * @since 1.2
     *
     * @see CoreApp
     *
     * @param bUploadingFile true or false
     *
     * @return void
     * @author dwinsor
     *
     * @deprecated do not expose
     */
    public static void setUploadingFile(boolean bUploadingFile)
    {
        ShellApp.getInstance().getCoreApp().setUploadingFile(bUploadingFile) ;
    }


    /**
     * gets major version string and build number
     *
     * @since 1.2
     *
     * @see XpressaSettings
     * @see Shell
     *
     * @param versionString major string version (e.g. 1.2.0.0)
     * @param iBuildNumber build number (duh) (e.g. 465)
     *
     * @return void
     * @author dwinsor
     *
     * @deprecated do not expose
     */
    public static String getCoreAppVersion()
    {
       int ibuildNumber = 0;
       String versionString = "";

       XpressaSettings settings = Shell.getXpressaSettings() ;

        // Get the Software revision
        versionString = settings.getVersion() ;
        if (versionString == null)
        {
            versionString = "Unknown" ;
        }
        else
            ibuildNumber = settings.getBuildNumber() ;

        String strSoftwareRevision = VersionUtils.buildVersionString(versionString, ibuildNumber) ;

        return strSoftwareRevision;
    }


    /**
     * returns free memory in java
     *
     * @since 1.2
     *
     *
     * @return memory free
     * @author dwinsor
     *
     * @deprecated do not expose
     */
     public static long getJVMFreeMemory()
    {
        return Runtime.getRuntime().freeMemory();
    }

    /**
     * returns total free memory in java
     *
     * @since 1.2
     *
     *
     * @return totalmemory free
     * @author dwinsor
     *
     * @deprecated do not expose
     */
    public static long getJVMTotalMemory()
    {
        return Runtime.getRuntime().totalMemory();
    }

  /**
     * flushCache
     * flushes cache.ser
     *
     * @since 1.2
     *
     *
     * @return void
     * @author dwinsor
     *
     * @deprecated do not expose
     *
     */
    public static void flushCache()
    {
        System.out.print("Flushing cache.ser...");
        Repository repository = Repository.getInstance();
        repository.flush();
        System.out.println("done.");
    }

    /**
     * @return 0 if OK was pressed otherwise anything else
     */
    protected static int doDisplayAuthorizeProfiles(int passwordAttempts, String[] userInfo)
    {
        AuthorizeProfilesForm form = new AuthorizeProfilesForm(ShellApp.getInstance().getCoreApp());
        form.setPasswordAttempts(passwordAttempts);
        form.setUserInfo(userInfo);
        int rc = form.showModal();

        return rc;
    }


    protected static int doDisplayMessageBox(int iType, String strTitle, String strText, boolean bBlocking)
    {
        //String strTitle = "Frog" ;
        // String strText = "dog dog dog" ;

        int iRC = -1 ;
        MessageBox msgBox = new MessageBox(ShellApp.getInstance().getCoreApp(), iType) ;

        msgBox.setTitle(strTitle) ;
        msgBox.setMessage(strText) ;

        if (bBlocking) {
            iRC = msgBox.showModal() ;
        } else {
            msgBox.showModeless() ;
        }
        return iRC ;
    }



    /**
     * @deprecated this should not be used and will be removed shortly
     * Note: this is currently used by the web UI
     */
    protected static void dropCallByID(String strCallId)
    {
        ShellApp shell = ShellApp.getInstance() ;

        // @TODO: We are just dropping the active call instead of what is necessary
        shell.getCoreApp().doDropActiveCall() ;
    }

//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////
}
