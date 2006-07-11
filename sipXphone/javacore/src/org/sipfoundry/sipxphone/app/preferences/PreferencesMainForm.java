/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/preferences/PreferencesMainForm.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.app.preferences ;

import java.awt.* ;
import java.text.* ;
import java.io.File ;
import java.util.Calendar ;

import org.sipfoundry.sipxphone.app.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.user.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.* ;
import org.sipfoundry.sipxphone.sys.app.core.* ;
import org.sipfoundry.sipxphone.sys.app.shell.* ;
import org.sipfoundry.sipxphone.sys.app.ShellApp;
import org.sipfoundry.sipxphone.sys.ApplicationRegistry;

import org.sipfoundry.sipxphone.service.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;
import org.sipfoundry.sipxphone.app.adjustvolume.* ;
import org.sipfoundry.sipxphone.app.CoreAboutbox ;

import org.sipfoundry.sipxphone.app.preferences.linemanager.* ;


/**
 *
 */
public class PreferencesMainForm extends PApplicationForm
        implements PFormButtonHook, PActionListener

{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final String RESOURCE_FILE = "PreferencesMainForm.properties" ;

    /** Default inactity timeout for admin access */
    private static final int INACTIVTY_DELAY_MS = 5*60*1000 ;   // 5 Minutes

/////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** container that holds list of preferences */
    protected PList m_listPrefs ;

    /** list model for container list */
    protected PDefaultListModel m_listModel ;

    /** dispatches commands to various onXXX handlers */
    protected icCommandDispatcher m_dispatcher = new icCommandDispatcher() ;

    /** Has this user been authenticated? */
    protected boolean m_bHasAuthenticated ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * stock constructor requiring an application context
     */
    public PreferencesMainForm(Application application)
    {
        super(application, "Preferences") ;

        setStringResourcesFile(RESOURCE_FILE) ;
        setTitle(getString("lblPreferencesMainTitle")) ;
        setIcon(getImage("imgPrefsAppsIcon")) ;
        setHelpText(getString("preference_category"), getString("preference_category_title")) ;

        initComponents() ;
        populateComponents() ;
        layoutComponents() ;
        initMenubar() ;

        m_bHasAuthenticated = false ;

        addFormButtonHook(this, true) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////


    public void doLayout()
    {
        m_listPrefs.doLayout() ;

        super.doLayout() ;
    }




    public void selectCategory(String strCategory)
    {
        // Call Handling
        if (strCategory.equals(getString("lblCallHandling")))
        {
            CallHandlingForm frmHandling = new CallHandlingForm(this) ;
            frmHandling.showModal() ;
        }
        // Xpressa Web Server
        else if (strCategory.equals(getString("lblWebServer")))
        {
            if (doAuthentication(false)) {
                WebServerForm frmWebServer = new WebServerForm(this, !m_bHasAuthenticated) ;
                frmWebServer.showModal() ;
            }
        }
        // User Maintenance
        else if (strCategory.equals(getString("lblUserMaintenance")))
        {
            if (doAuthentication(true)) {
                UserMaintenanceForm form = new UserMaintenanceForm(this) ;
                form.showModal() ;
            }
        }
        // Volume & Contrast
        else if (strCategory.equals(getString("lblVolume")) ||
                 strCategory.equals(getString("lblVolume_softphone")))
        {
            SelectAdjustmentForm frmSelectAdjustment = new SelectAdjustmentForm(this) ;
            int iAdjustment = frmSelectAdjustment.showModal() ;

            if (iAdjustment != SelectAdjustmentForm.SEL_NONE)
            {
                int iContext = 0;

                switch (iAdjustment)
                {
                    case SelectAdjustmentForm.SEL_BASE:
                        iContext = AdjustVolumeForm.BaseSpeaker;
                        break ;
                    case SelectAdjustmentForm.SEL_CONTRAST:
                        iContext = AdjustVolumeForm.Contrast;
                        break ;
                    case SelectAdjustmentForm.SEL_HANDSET:
                        iContext = AdjustVolumeForm.HandsetSpeaker;
                        break ;
                    case SelectAdjustmentForm.SEL_RINGER:
                        iContext = AdjustVolumeForm.Ringer;
                        break ;
                    case SelectAdjustmentForm.SEL_HEADSET:
                        iContext = AdjustVolumeForm.HeadsetSpeaker;
                        break ;
                }

                AdjustVolumeForm frmAdjustVol = new AdjustVolumeForm(this, false) ;
                frmAdjustVol.setContext(iContext) ;
                frmAdjustVol.showModal() ;
            }
        }
        else if (strCategory.equals(getString("lblLineManager")))
        {
            // Line manager does not require authentication
            LineManagerForm frmLineManager = new LineManagerForm(this, true) ;
            frmLineManager.showModal() ;
        }
        else if (strCategory.equals(getString("lblLocalPrefs")))
        {
            // Prefs stored only localy on this device (e.g. local-config)
            LocalPrefsForm frmLocal = new LocalPrefsForm(this);
            frmLocal.showModal();
        }
        else if (strCategory.equals("No Guest Access"))
        {
            //do something here
            int iExitCode =PSecurityManager.getInstance().showAuthenticationForm(this);
            if( iExitCode == SimpleTextInputForm.OK )
            {
                populateComponents() ;
            }
        }
        else if (strCategory.equals("Software Revision"))
        {
            //do something here.
            SystemAboutBox box = new SystemAboutBox(PreferencesMainForm.this);
            box.onAbout();
        }
        else if (strCategory.equals("Restart"))
        {
            //do something here.
            MessageBox messageBox = new MessageBox(getApplication(), MessageBox.TYPE_WARNING) ;
            messageBox.setMessage(getString("lblWarningRestart")) ;

            if (messageBox.showModal() == MessageBox.OK) {
                PingerInfo.getInstance().reboot() ;
            }
        }
        else if (strCategory.equals("Factory Defaults"))
        {
          onFactoryDefaults();
        }
        else if (strCategory.equals("Logout Admin Mode"))
        {
            if( PSecurityManager.getInstance().isInGuestFeaturesMode() )
            {
                PSecurityManager.getInstance().showGuestAdminFeatures(false);
            }
            populateComponents() ;

        }
    }
    
    
    /**
     * This method is invoked as a response to the "Save SIP Log" menu item.
     * This method will dump the contents of the phone's SIP log to disk.
     */    
    public void onSaveSipLog()
    {
        PingerInfo info = PingerInfo.getInstance() ;
        
        String strFilename = Settings.getString("SIPLOG_NAME", "siplog-{0}.txt") ;
        SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd-HHmmss") ;
        String strDateTime = sdf.format(Calendar.getInstance().getTime()) ;
        

        Object[] objs = {strDateTime};
        strFilename = MessageFormat.format(strFilename, objs) ;                          
        String strFilespec = info.getFlashFileSystemLocation() + 
                File.separator +
                strFilename ;
                                                                           
        if (info.saveSipLog(strFilespec))
        {
            Object params[] = {strFilename} ;            
            String strMsg = getString("msgSaveSIPLogSuccess") ;
            strMsg = MessageFormat.format(strMsg, params) ;
            
            MessageBox msgBox = new MessageBox(this) ;
            msgBox.setType(MessageBox.TYPE_INFORMATIONAL) ;
            msgBox.setMessage(strMsg) ;
            msgBox.showModal() ;                        
        }
        else
        {
            Object params[] = {strFilename} ;            
            String strMsg = getString("msgSaveSIPLogFailure") ;
            strMsg = MessageFormat.format(strMsg, params) ;
                        
            MessageBox msgBox = new MessageBox(this) ;
            msgBox.setType(MessageBox.TYPE_ERROR) ;
            msgBox.setMessage(strMsg) ;
            msgBox.showModal() ;                        
        }            
    }
    
    
    public void onEnableSipLog(boolean bEnable)
    {
        PingerInfo info = PingerInfo.getInstance() ;
        info.enableSipLog(bEnable) ;
        updateSipLogMenuEntries() ;
        
        String strMsg ;
        
        if (bEnable)
        {
            strMsg = getString("msgEnableSIPLog") ;
        }
        else
        {
            strMsg = getString("msgDisableSIPLog") ;
        }
        
        MessageBox msgBox = new MessageBox(this) ;
        msgBox.setType(MessageBox.TYPE_INFORMATIONAL) ;
        msgBox.setMessage(strMsg) ;
        msgBox.showModal() ;          
    }


    /**
     * This method is invoked as a reponse to the "Reset" menu item.  This
     * method forces a reset of flash and returns the phone to factory default
     * settings.
     * <p>
     * A confirmation message warns the end user before resetting the phone.
     */
    public void onFactoryDefaults()
    {
        MessageBox messageBox = new MessageBox(this, MessageBox.TYPE_WARNING) ;
        messageBox.setMessage(getString("lblWarningRestoreFactoryDefaults")) ;

        if (messageBox.showModal() == MessageBox.OK) {
            PingerInfo.getInstance().restoreFactoryDefaults() ;
            PingerInfo.getInstance().rebootNoFlush() ;
        }
    }


    public boolean buttonEvent(PButtonEvent event, PForm form)
    {
        restartInactivityTimer(false) ;

        return false ;
    }


    public void actionEvent(PActionEvent event)
    {
        if( PSecurityManager.getInstance().isInGuestFeaturesMode() )
        {
            PSecurityManager.getInstance().showGuestAdminFeatures(false);
        }
        closeForm(-1) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /**
     * Restarts an inactivity timer for the purpose of closing the preferences
     * form after some period of inactivity.  This timer is not set unless,
     * an admin password was entered.
     */
    protected void restartInactivityTimer(boolean bClear)
    {
        if (bClear)
        {
            System.out.println("Clearing inactivity timer") ;
            Timer timer = Timer.getInstance() ;
            timer.removeTimers(this);
        }
        else if (m_bHasAuthenticated)
        {
            PingerConfig config = PingerConfig.getInstance() ;
            int iDelay = INACTIVTY_DELAY_MS ;

            // Get the configurate inactivity timeout
            String strValue = config.getValue(
                    PingerConfig.PHONESET_PREFS_INACTIVITY_TIMEOUT) ;
            if (strValue != null)
            {
                try
                {
                    iDelay = Integer.parseInt(strValue) * 1000 ;
                }
                catch (NumberFormatException nfe) { }
            }

            if (iDelay <= 0)
                return ;

            // Reset the timer
            Timer timer = Timer.getInstance() ;
            timer.resetTimer(iDelay, this, null) ;
        }
    }

    
    /**
     * Update all of the Sip Log menu entries to reflect the current state.
     */ 
    protected void updateSipLogMenuEntries()
    {
        PMenuComponent menu = getRightMenuComponent() ;
        PActionItem items[] = menu.getItems() ;
        
        /*
         * First, remove any existing the menu items
         */
        int i, deleteIndex ;
        for (i=0, deleteIndex=0; 
                i<items.length; 
                i++, deleteIndex++)
        {
            if (    (items[i].getActionCommand().equals(m_dispatcher.ACTION_DISABLESIPLOG)) 
                ||  (items[i].getActionCommand().equals(m_dispatcher.ACTION_ENABLESIPLOG))
                ||  (items[i].getActionCommand().equals(m_dispatcher.ACTION_SAVESIPLOG)))
            {
                menu.removeItemAt(deleteIndex--) ;
            }                        
        }
            
        /**
         * Next, re-add all of the sip log entries
         */        
        // Add "Save SIP Log" if under instant xpressa
        if (Shell.getXpressaSettings().getProductID() == 
                XpressaSettings.PRODUCT_INSTANTXPRESSA)
        {
            boolean bEnableSaveSIPLog = PingerInfo.getInstance().isSipLogEnabled() ;                         
            
            if (bEnableSaveSIPLog)
            {
                // If enabled, offer ability to disable
                PActionItem actionDisableSIPLog = new PActionItem(new PLabel(getString("lblDisableSIPLog")),
                         getString("hint/preferences/select/disableSIPLog"),
                         m_dispatcher,
                         m_dispatcher.ACTION_DISABLESIPLOG) ;
                        
                 getRightMenuComponent().addItem(actionDisableSIPLog) ;                        
                    
            }
            else
            {
                // If the syslog is disabled, offer the ability to enable it.
                PActionItem actionEnableSIPLog = new PActionItem(new PLabel(getString("lblEnableSIPLog")),
                        getString("hint/preferences/select/enableSIPLog"),
                        m_dispatcher,
                        m_dispatcher.ACTION_ENABLESIPLOG) ;
                        
                getRightMenuComponent().addItem(actionEnableSIPLog) ;                
            }
            
            // Add the ability to store SIP Log
            PActionItem actionSaveSIPLog = new PActionItem(new PLabel(getString("lblSaveSIPLog")),
                    getString("hint/preferences/select/saveSIPLog"),
                    m_dispatcher,
                    m_dispatcher.ACTION_SAVESIPLOG) ;
                    
            getRightMenuComponent().addItem(actionSaveSIPLog) ;

            // Lastly, disable the "Save SIP Log" entry if the sip log is not enabled.
            enableMenusByAction(bEnableSaveSIPLog, m_dispatcher.ACTION_SAVESIPLOG) ;
        }        
    }

    protected void initMenubar()
    {
        // Initialize Bottom Menu

        PBottomButtonBar menuControl = getBottomButtonBar() ;

        PActionItem actionAbout = new PActionItem( new PLabel("About"),
                getString("hint/core/system/aboutform"),
                    m_dispatcher,
                    m_dispatcher.ACTION_ABOUT) ;
        getRightMenuComponent().addItem(actionAbout) ;


        getRightMenuComponent().addItem(new PActionItem(
                new PLabel("Factory Defaults"),
                getString("hint/core/system/factory_defaults"),
                m_dispatcher,
                m_dispatcher.ACTION_RESET)) ;

        PActionItem actionAdjust = new PActionItem( new PLabel("Adjust"),
                getString("hint/preferences/select/adjust"),
                m_dispatcher,
                m_dispatcher.ACTION_SET) ;
        menuControl.setItem(PBottomButtonBar.B3, actionAdjust) ;
        getLeftMenuComponent().addItem(actionAdjust) ;

        PActionItem actionExit = new PActionItem(new PLabel("Exit"),
                getString("hint/core/system/exitform"),
                m_dispatcher,
                m_dispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, actionExit) ;
        getLeftMenuComponent().addItem(actionExit) ;       
        
        updateSipLogMenuEntries() ;             
    }



    protected boolean doAuthentication(boolean bRequire)
    {
        boolean bContinue = true ;

        if (!m_bHasAuthenticated) {
            icPasswordAuthentication form = new icPasswordAuthentication(this) ;
            if (form.showModal() == form.OK)
            {
                m_bHasAuthenticated = true ;
                restartInactivityTimer(false) ;
            }
            else if (bRequire == true)
            {
                bContinue = false ;
            }
            else
            {
                MessageBox messageBox = new MessageBox(this, MessageBox.TYPE_WARNING) ;
                messageBox.setMessage(getString("warnAuthenticationFailed")) ;
                if( messageBox.showModal() == MessageBox.CANCEL )
                    bContinue = false;
            }
        }
        return bContinue ;
    }


    protected void initComponents()
    {
        // Create new list
        m_listModel = new PDefaultListModel() ;
        m_listPrefs = new PList(m_listModel) ;
        m_listPrefs.addActionListener(m_dispatcher) ;
        m_listPrefs.setItemRenderer(new PIconTextRenderer()) ;
    }


    /**
     * <pre>
     * Hack done to accomodate Hotel Commonwealth features:
     * Launching Admin/Preferences in user mode lists the following
       options:
      a) Software Revision
      c) Admin Mode (requires password)
      d) Volume [& Contrast]

    Software Revision displays the software versions and copyrights,
    but does *not* display their SIP URL or "Info" button unless
    in Admin mode (see below).

    Admin Mode places the user into "Admin" mode after they
    successfully enter a built-in password.  It is important that this
    mode is accessible even if the configuration is corrupted. Placing
    this password in the locale/site specific software file may be the
    best choice.  It also reduces the security exposure.

    Admin mode would timeout after some period of inactivity.

    Once enabled, this mode:
      a) Enables the rest (or a selection) of the preferences applets.
         The user will still need to enter their admin password to
         change their settings.
      b) Adds "Restore to Factory Defaults", restart, version check to the Admin options.
      c) Enables the "Info" button and SIP URL in the About box, so
         admins can discover the IP address.
      d) A method of disabling Admin Mode
      </pre>
     */
    protected void populateComponents()
    {
       m_listModel.removeAllElements();

        //determine if call handling shows
        boolean bCanShowCallHandlingMenu = true;
        String strCallHandlingCanShow = PingerConfig.getInstance().getValue("PHONESET_PREFS_CALL_HANDLING");
        if (strCallHandlingCanShow != null && strCallHandlingCanShow.equalsIgnoreCase("DISABLE"))
        {
            bCanShowCallHandlingMenu = false;
        }

        // Add components to list
        boolean bGuestMode = PSecurityManager.getInstance().isInGuestFeaturesMode();
        boolean bShowGuestAdminFeatures = PSecurityManager.getInstance().shouldShowGuestAdminFeatures();
        boolean bRestricted = false;
        if( bGuestMode && !bShowGuestAdminFeatures )
            bRestricted = true;

        if( !bRestricted )
        {
            if (bCanShowCallHandlingMenu)
            {
                m_listModel.addElement(new PIconTextRendererData(
                        getString("lblCallHandling"),
                        getImage("imgCallHandling")),
                        getString("hint/preferences/select/call_handling")) ;
            }
        }

        // only if soft phone
        m_listModel.addElement(new PIconTextRendererData(
                getString("lblVolume_softphone"),
                getImage("imgVolumeAndContrast")),
                getString("hint/preferences/select/volume_contrast")) ;

        // Softphone only because currently the only setting in
        // here is specific to softphone.
        m_listModel.addElement(new PIconTextRendererData(
                getString("lblLocalPrefs"),
                getImage("imgUserMaintenance")),
                getString("hint/preferences/select/local_prefs")) ;

        if( !bRestricted )
        {
            m_listModel.addElement(new PIconTextRendererData(
                    getString("lblWebServer"),
                    getImage("imgXpressaWeb")),
                    getString("hint/preferences/select/web")) ;

            m_listModel.addElement(new PIconTextRendererData(
                    getString("lblUserMaintenance"),
                    getImage("imgUserMaintenance")),
                    getString("hint/preferences/select/user_maintenance")) ;

            m_listModel.addElement(new PIconTextRendererData(
                    getString("lblLineManager"),
                    getImage("imgUserMaintenance")),
                    getString("hint/preferences/select/line_manager")) ;
        }

        if( bGuestMode )
        {
               m_listModel.addElement(new PIconTextRendererData(
                    "Software Revision",
                    getImage(TaskApplicationList.DEFAULT_APP_ICON)),
                    getString("hint/core/system/aboutform")) ;
                if( bShowGuestAdminFeatures )
                {
                    m_listModel.addElement(new PIconTextRendererData(
                    getString("Factory Defaults"),
                    getImage(TaskApplicationList.DEFAULT_APP_ICON)),
                    getString("hint/core/system/factory_defaults"));

                    m_listModel.addElement(new PIconTextRendererData(
                        getString("Restart"),
                        getImage(TaskApplicationList.DEFAULT_APP_ICON)),
                        getString("hint/core/system/restart")) ;

                    m_listModel.addElement(new PIconTextRendererData(
                        getString("Logout Admin Mode"),
                        getImage(TaskApplicationList.DEFAULT_APP_ICON)),
                        getString("Logs you off from Admin mode.")) ;
                }else
                {
                    m_listModel.addElement(new PIconTextRendererData(
                    getString("No Guest Access"),
                    getImage(TaskApplicationList.DEFAULT_APP_ICON)),
                    getString("Lets you access certain admin features.")) ;
                }
        }
        super.doLayout();

    }


    protected void layoutComponents()
    {
        addToDisplayPanel(m_listPrefs, new Insets(1, 1, 1, 1)) ;
    }


    protected void clearComponents()
    {
        removeFromDisplayPanel(m_listPrefs) ;
    }


    /**
     *
     */
    protected void onSet()
    {
        // Get the list item text.  We may need to pull this out of the
        // encapsulating PIconTextRendererData object.
        Object objSelectedElementText = m_listPrefs.getSelectedElement() ;
        if (objSelectedElementText instanceof PIconTextRendererData)
        {
            objSelectedElementText = ((PIconTextRendererData)objSelectedElementText).getText() ;
        }

        // Kick out if we don't have any selected data
        if (objSelectedElementText != null)
        {
            selectCategory(objSelectedElementText.toString()) ;
        }
    }


    /**
     *
     */
    protected void onCancel()
    {
        restartInactivityTimer(true) ;
        closeForm() ;
    }



//////////////////////////////////////////////////////////////////////////////
// Inner classes
////
   /**
     *
     */
    public class icCommandDispatcher implements PActionListener
    {
        public final String ACTION_CANCEL        = "action_cancel" ;
        public final String ACTION_SET           = "action_set" ;
        public final String ACTION_ABOUT         = "action_about" ;
        public final String ACTION_RESET         = "action_reset" ;
        public final String ACTION_SAVESIPLOG    = "action_savesiplog" ;
        public final String ACTION_ENABLESIPLOG  = "action_enablesiplog" ;
        public final String ACTION_DISABLESIPLOG = "action_disablesiplog" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_CANCEL))
            {
                onCancel() ;
            }
            else if (event.getActionCommand().equals(ACTION_SET))
            {
                onSet() ;
            }
            else if (event.getActionCommand().equals(PList.ACTION_DOUBLE_CLICK))
            {
                onSet() ;
            }
            else if (event.getActionCommand().equals(ACTION_ABOUT))
            {
                SystemAboutBox box = new SystemAboutBox(PreferencesMainForm.this);
                box.onAbout();
            }
            else if (event.getActionCommand().equals(ACTION_RESET))
            {
                onFactoryDefaults() ;
            }
            else if (event.getActionCommand().equals(ACTION_SAVESIPLOG))
            {
                onSaveSipLog() ;
            }
            else if (event.getActionCommand().equals(ACTION_ENABLESIPLOG))
            {
                onEnableSipLog(true) ;
            }
            else if (event.getActionCommand().equals(ACTION_DISABLESIPLOG))
            {
                onEnableSipLog(false) ;
            }            
        }
    }

    public class icPasswordAuthentication extends SimpleTextInputForm
    {
        private icPACommandDispatcher m_dispatcher ;

        /** tip for the password form */
        private String m_strInstructions = "" ;

        /** the number of dial by url tips */
        protected int        m_iNumTips = - 1 ;

        public icPasswordAuthentication(PForm formParent)
        {
            super(formParent, "Authentication") ;

            m_dispatcher = new icPACommandDispatcher() ;

            setLabel("admin password: ") ;
            setMode(icPasswordAuthentication.MODE_PASSWORD_TEXT) ;
            this.setHelpText(this.getString("admin"),
                             this.getString("admin_title")) ;
            nextTip();
            this.setInstructions(m_strInstructions);
            initMenus() ;
        }


        /**
         * loads the next tip.
         */
        protected void nextTip()
        {
              // Load number of tips if not already known
              if (m_iNumTips == -1) {
                  try {
                      m_iNumTips = Integer.parseInt(this.getString("lblNumDialByURLTips")) ;
                  } catch (Exception e) { /* burp */ }
              }

              int iTip = (int) (Math.random() * m_iNumTips) ;
              // Cycle if we have enough...
              if (m_iNumTips > 0) {
                  m_strInstructions = this.getString("lblDialByURLTip" + (iTip+1)) ;
              }
        }

        /**
         * on focus gained of this password authentication form,
         * load the next tip and show the tip.
         */
        public void onFocusGained(PForm formLosingFocus)
        {
            nextTip();
            this.setInstructions(m_strInstructions);
        }


        /**
         *
         * @returns
         */
        private void initMenus()
        {
            this.getLeftMenuComponent().removeAllItems() ;
            this.getRightMenuComponent().removeAllItems() ;

            PBottomButtonBar menuControl = this.getBottomButtonBar() ;

            PActionItem actionAbout = new PActionItem( new PLabel("About"),
                this.getString("hint/core/system/aboutform"),
                    m_dispatcher,
                    m_dispatcher.ACTION_ABOUT) ;
            this.getRightMenuComponent().addItem(actionAbout) ;

            PActionItem actionOk = new PActionItem( new PLabel(this.getString("lblGenericOk")),
                    this.getString("hint/preferences/authentication/ok"),
                    m_dispatcher,
                    m_dispatcher.ACTION_OK) ;
            menuControl.setItem(PBottomButtonBar.B3, actionOk) ;
            this.getLeftMenuComponent().addItem(actionOk) ;

            PActionItem actionCancel = new PActionItem
                ( new PLabel(this.getString("lblGenericCancel")),
                  this.getString("hint/preferences/authentication/cancel"),
                  m_dispatcher,
                  m_dispatcher.ACTION_CANCEL) ;
            menuControl.setItem(PBottomButtonBar.B2, actionCancel) ;
            this.getLeftMenuComponent().addItem(actionCancel) ;
        }


        public void onOk()
        {
            PUserManager userManager = new PUserManager() ;

            String strPasswd = getText() ;
            try {
                if (userManager.authenticateUser("admin", strPasswd)) {
                    super.onOk() ;
                } else {
                    setText("") ;
                    setInstructions("Invalid Password.  Please re-enter your password and try again.") ;
                }
            } catch (Exception e) {
                MessageBox error = new MessageBox(this, MessageBox.TYPE_ERROR) ;
                error.setMessage(this.getString("lblAuthenticationError") + "\n" + e.toString()) ;
                error.showModal() ;
                super.onOk() ;
            }
        }

        public void onCancel()
        {
            super.onCancel() ;
        }

        private class icPACommandDispatcher implements PActionListener
        {
            public final String ACTION_OK     = "ACTION_OK" ;
            public final String ACTION_CANCEL = "ACTION_CANCEL" ;
            public final String ACTION_ABOUT  = "ACTION_ABOUT" ;
            
            public void actionEvent(PActionEvent event)
            {
                if (event.getActionCommand().equals(ACTION_OK)) {
                    onOk() ;
                } else if (event.getActionCommand().equals(ACTION_CANCEL)) {
                    onCancel() ;
                } else if (event.getActionCommand().equals(ACTION_ABOUT)) {
                    SystemAboutBox box = new SystemAboutBox(PreferencesMainForm.this);
                    box.onAbout();
                }
            }
        }
    }
}







