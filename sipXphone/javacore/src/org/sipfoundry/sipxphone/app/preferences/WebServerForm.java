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


package org.sipfoundry.sipxphone.app.preferences ;

import javax.telephony.* ;

import java.awt.Insets ;
import java.util.Enumeration ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;


/**
 * The WebServerForm allows users to enable/disable the web server along with
 * setting the web server port.  Now, there is a bit of magic going on here.
 * the configuration param that controls the web server is hidden and the
 * default values depend on the hardware platform.  The hardphone defaults
 * to web server enabled on port 80 and the softphone defaults to a disabled
 * web server.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class WebServerForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** return codes from Show Modal: Cancel, do do anything */
    public final static int CANCEL  = 0 ;
    /** return codes from Show Modal: Dial */
    public final static int SET    = 1 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** container that houses the controls */
    protected PScrollableComponentContainer m_contDialogue ;

    /** Enable Non Secure Checkbox */
    protected PCheckbox m_chkEnableNonSecureWebServer ;

    /** Enable Secure Checkbox */
    protected PCheckbox m_chkEnableSecureWebServer ;

    /** Web Server non secure Port */
    protected PTextField m_tfNonSecurePort ;

    /** Web Server secure Port */
    protected PTextField m_tfSecurePort ;

    /** Command bar displayed on right of screen. */
    private PCommandBar m_cbActions ;

    /** Automatically enables/disables command bar buttons passed on focus */
    private TextFieldActionEventAdapter m_textfieldAdapter ;

    /** dispatches commands to various onXXX handlers */
    protected icCommandDispatcher m_dispatcher = new icCommandDispatcher() ;

    /** Has the user made any changes? */
    protected boolean m_bMadeChanges ;

    /** Is this form opened in a read only mode? */
    protected boolean m_bReadOnly ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * stock constructor requiring a parent form
     */
    public WebServerForm(PForm formParent, boolean bReadOnly)
    {
        super(formParent, "sipXphone Web") ;
        setStringResourcesFile("WebServerForm.properties") ;
        setTitle(getString("lblWebServer")) ;
        setIcon(getImage("imgPrefsAppsIcon")) ;

        m_bReadOnly = bReadOnly ;

        initComponents() ;
        layoutComponents() ;
        initMenubar() ;

        setHelpText(getString("preference_webserver"), getString("preference_webserver_title")) ;
        m_bMadeChanges = false ;
    }


    /**
     * Looks at our configuration information to determine if the webserver is
     * enabled.
     */
    protected boolean isConfigNonSecureWebserverEnabled()
    {
        boolean bRC = false ;
        String strPort = PingerConfig.getInstance().getValue(PingerConfig.PHONESET_HTTP_PORT) ;
        if ((strPort == null) || (strPort.length() == 0) || strPort.equals("-1")) {
            // Apply "Known" defaults
            if (PingerApp.isTestbedWorld()) {
                bRC = false ;
            } else {
                bRC = true ;
            }
        } else {
            if (CheckTextfieldUtils.validPort(strPort))
                bRC = true ;
        }
        return bRC ;
    }
    /**
     * Looks at our configuration information to determine if the webserver is
     * enabled.
     */
    protected boolean isConfigSecureWebserverEnabled()
    {
        boolean bRC = false ;
        String strPort = PingerConfig.getInstance().getValue(PingerConfig.PHONESET_HTTPS_PORT) ;
        if ((strPort == null) || (strPort.length() == 0) || strPort.equals("-1")) {
            // Apply "Known" defaults
            if (PingerApp.isTestbedWorld()) {
                bRC = false ;
            } else {
                bRC = true ;
            }
        } else {
            if (CheckTextfieldUtils.validPort(strPort))
                bRC = true ;
        }
        return bRC ;
    }


    /**
     * Looks at our configuration information to determine what port webserver
     * is bound to.
     */
    protected String getConfigWebserverNonSecurePort()
    {
        String strRC = "80" ;

        String strPort = PingerConfig.getInstance().getValue(PingerConfig.PHONESET_HTTP_PORT) ;
        if ((strPort != null) && (strPort.length() != 0) && CheckTextfieldUtils.validPort(strPort)) {
            strRC = strPort ;
        }
        return strRC ;
    }

    /**
     * Looks at our configuration information to determine what port webserver
     * is bound to.
     */
    protected String getConfigWebserverSecurePort()
    {
        String strRC = "443" ;

        String strPort = PingerConfig.getInstance().getValue(PingerConfig.PHONESET_HTTPS_PORT) ;
        if ((strPort != null) && (strPort.length() != 0) && CheckTextfieldUtils.validPort(strPort)) {
            strRC = strPort ;
        }
        return strRC ;
    }

    protected void initComponents()
    {
        // Create our scrollable container
        m_contDialogue  = new PScrollableComponentContainer(4, PButtonEvent.BID_R1) ;
        m_contDialogue.setBounds(28, 2, 129, 104) ;

        // Init Action ButtonBar
        m_cbActions = new PCommandBar() ;
        m_cbActions.setBounds(0, 0, 160, 108) ;

        // Create Components
        m_chkEnableNonSecureWebServer = new PCheckbox(getString("lblEnableNonSecureWebServer"), isConfigNonSecureWebserverEnabled()) ;
        m_chkEnableNonSecureWebServer.setHint(getString("hint/preference/webserver/enable_nonsecure_server")) ;
        m_chkEnableNonSecureWebServer.addItemListener(new icCheckboxChangedListener()) ;

/* PUT BACK WHEN WE ENABLE SSL
        m_chkEnableSecureWebServer = new PCheckbox(getString("lblEnableSecureWebServer"), isConfigSecureWebserverEnabled()) ;
        m_chkEnableSecureWebServer.setHint(getString("hint/preference/webserver/enable_secure_server")) ;
        m_chkEnableSecureWebServer.addItemListener(new icCheckboxChangedListener()) ;
*/
        m_tfNonSecurePort = new PTextField(getConfigWebserverNonSecurePort()) ;
        m_tfNonSecurePort.setLabel(getString("lblNonSecurePort"), PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST) ;
        m_tfNonSecurePort.setAlphanumericMode(false) ;
        m_tfNonSecurePort.addTextListener(new icTextListener()) ;

/* PUT BACK WHEN WE ENABLE SSL
        m_tfSecurePort = new PTextField(getConfigWebserverSecurePort()) ;
        m_tfSecurePort.setLabel(getString("lblSecurePort"), PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST) ;
        m_tfSecurePort.setAlphanumericMode(false) ;
        m_tfSecurePort.addTextListener(new icTextListener()) ;
*/
        m_textfieldAdapter = new TextFieldActionEventAdapter(m_tfNonSecurePort, m_cbActions, false) ;

/* PUT BACK WHEN WE ENABLE SSL

         m_textfieldAdapter.addTextfield(m_tfSecurePort,false);
*/

        m_textfieldAdapter.populateCommandbar() ;


        // Add components to scrollable container
        m_contDialogue.addComponent(m_chkEnableNonSecureWebServer) ;

/* PUT BACK WHEN WE ENABLE SSL
        m_contDialogue.addComponent(m_chkEnableSecureWebServer) ;
*/
        onEnableNonSecureWebServerChanged() ;
/* PUT BACK WHEN WE ENABLE SSL
        onEnableSecureWebServerChanged() ;
*/
    }


    protected void layoutComponents()
    {
        PContainer container = new PContainer() ;

        container.setLayout(null) ;

        m_cbActions.setBounds(0, 0, 160, 108) ;
        container.add(m_cbActions) ;

        m_contDialogue.setBounds(28, 2, 129, 104) ;
        container.add(m_contDialogue) ;

        addToDisplayPanel(container, new Insets(0, 0, 0, 0)) ;
    }


    protected void initMenubar()
    {
        // Initialize Bottom Menu
        PBottomButtonBar menuControl = getBottomButtonBar() ;
        PMenuComponent menuComponent = getLeftMenuComponent() ;

        PActionItem itemOK = new PActionItem(
                new PLabel(getString("lblGenericOk")),
                getString("hint/preferences/common/ok"),
                m_dispatcher,
                m_dispatcher.ACTION_SET) ;
        menuControl.setItem(PBottomButtonBar.B3, itemOK) ;
        menuComponent.addItem(itemOK) ;

        PActionItem itemCancel = new PActionItem(
                new PLabel("Cancel"),
                getString("hint/preferences/common/cancel"),
                m_dispatcher,
                m_dispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, itemCancel) ;
        menuComponent.addItem(itemCancel) ;

        enableMenusByAction(!m_bReadOnly, m_dispatcher.ACTION_SET) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Invoked when the user has finished making changes and ready to commit.
     */
    protected void onSet()
    {
        boolean bSuccess = true ;

        if (m_bMadeChanges) {
            if (m_chkEnableNonSecureWebServer.getState()) {
                String strPort = m_tfNonSecurePort.getText() ;
                if (!CheckTextfieldUtils.validPort(strPort)) {
                    displayErrorForm(getString("errInvalidPort")) ;
                    bSuccess = false ;
                } else {
                    PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG, PingerConfig.PHONESET_HTTP_PORT, strPort) ;
                }
            } else {
                PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG, PingerConfig.PHONESET_HTTP_PORT, "0") ;
            }
            /* DWW TOOK OUT UNTIL HTTPS IS FLUSHED OUT MORE
            if (m_chkEnableSecureWebServer.getState()) {
                String strPort = m_tfSecurePort.getText() ;
                if (!CheckTextfieldUtils.validPort(strPort)) {
                    displayErrorForm(getString("errInvalidPort")) ;
                    bSuccess = false ;
                } else {
                    PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG, PingerConfig.PHONESET_HTTPS_PORT, strPort) ;
                }
            } else {
                PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG, PingerConfig.PHONESET_HTTPS_PORT, "0") ;
            }
            */
        }

        if (bSuccess) {
            if (m_bMadeChanges) {
                MessageBox msgBox = new MessageBox(this, MessageBox.TYPE_WARNING) ;
                msgBox.setMessage(getString("lblRestartWarning")) ;
                msgBox.setButton(MessageBox.B3,
                                 getString("lblRestart"),
                                 getString("hint/core/system/restart")) ;
                if (msgBox.showModal() == MessageBox.B3) {
                    PingerInfo.getInstance().reboot() ;
                }
            }
            closeForm(SET) ;
        }
    }


    protected void onCancel()
    {
        closeForm(CANCEL) ;
    }


    /**
     *
     */
    protected void onEnableNonSecureWebServerChanged()
    {
        if (m_chkEnableNonSecureWebServer.getState()) {
            m_contDialogue.insertComponentAfter(m_tfNonSecurePort, m_chkEnableNonSecureWebServer) ;
            m_tfNonSecurePort.setFocus(true) ;
        } else {
            m_contDialogue.removeComponent(m_tfNonSecurePort) ;
        }
    }

    /**
     *
     */
    protected void onEnableSecureWebServerChanged()
    {

/* DWW TOOK OUT UNTIL HTTPS IS FLUSHED OUT MORE

        if (m_chkEnableSecureWebServer.getState()) {
            m_contDialogue.insertComponentAfter(m_tfSecurePort, m_chkEnableSecureWebServer) ;
            m_tfSecurePort.setFocus(true) ;
        } else {
            m_contDialogue.removeComponent(m_tfSecurePort) ;
        }
*/
    }

    protected void displayErrorForm(String strErrorMsg)
    {
        SimpleTextForm frmError = new SimpleTextForm(this, getString("errTitle")) ;
        frmError.setText(strErrorMsg) ;
        frmError.showModal() ;

    }


//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////

    /**
     * Dispatches action commands to various onXXX handlers
     */
    private class icCommandDispatcher implements PActionListener
    {
        public final String ACTION_CANCEL    = "action_cancel" ;
        public final String ACTION_SET       = "action_set" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            }
            else if (event.getActionCommand().equals(ACTION_SET)) {
                onSet() ;
            }
        }
    }

    private class icCheckboxChangedListener implements PItemListener
    {
        public void itemStateChanged(PItemEvent event)
        {
            if (event.getSource().equals(m_chkEnableNonSecureWebServer)) {
                m_bMadeChanges = true ;
                onEnableNonSecureWebServerChanged() ;
            }
  /* DWW TOOK OUT UNTIL HTTPS IS FLUSHED OUT MORE
              if (event.getSource().equals(m_chkEnableSecureWebServer)) {
                m_bMadeChanges = true ;
                onEnableSecureWebServerChanged() ;

            }
*/
          }
    }

    private class icTextListener implements PTextListener
    {
        /**
         * This listener method is invoked when the value of the text has changed.
         *
         * @param PTextEvent The event object describing the state change.
         */
        public void textValueChanged(PTextEvent e)
        {
            m_bMadeChanges = true ;
        }


        /**
         * This listener method is invoked when the caret position is changed
         * <p><b>NOTE</b>: This is an extension to the AWT TextListener interface.
         *
         * @param PTextEvent The event object describing the state change.
         */
        public void caretPositionChanged(PTextEvent e) { }
    }
}
