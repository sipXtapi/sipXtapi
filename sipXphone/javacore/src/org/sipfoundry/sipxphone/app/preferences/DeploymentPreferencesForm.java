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

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;

import java.awt.Insets ;


/**
 *
 */
public class DeploymentPreferencesForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    /** return codes from Show Modal: Cancel, do do anything */
    public final static int CANCEL  = 0 ;
    /** return codes from Show Modal: Dial */
    public final static int DIAL    = 1 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** container that houses the controls */
    protected PScrollableComponentContainer m_contDialogue ;

    /** Whether using deployment server*/
    protected PCheckbox     m_chkDeployment ;
    /** Ask next time start form*/
    protected PCheckbox     m_chkAsk;
    /** Deployment Server */
    protected ParameterTextField m_txtServer ;
    /** Are we using deployment server? */
    protected boolean m_bDeployment ;
    /** Initial value of deployment server */
    protected String m_strDeploymentInitial ;
    protected boolean m_bDeploymentInitial ;


    /** Action Button Bar */
    protected PCommandBar m_bbActions ;
    /** dispatches commands to various onXXX handlers */
    protected icCommandDispatcher m_dispatcher = new icCommandDispatcher() ;
    /** calling application */

    protected boolean m_bReadOnly ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * stock constructor requiring a parent form
     */
    public DeploymentPreferencesForm(PForm formParent, boolean bReadOnly)
    {
        super(formParent, "Deployment Preferences") ;
        m_bReadOnly = bReadOnly ;


        setStringResourcesFile("DeploymentPreferencesForm.properties") ;
        setTitle(getString("lblDeploymentPreferencesTitle")) ;
        setIcon(getImage("imgPrefsAppsIcon")) ;

        initComponents() ;
        layoutComponents() ;
        initMenubar() ;

        setHelpText(getString("preference_deployment"), getString("preference_deployment_title")) ;
    }


    protected void initComponents()
    {
        // Create our scrollable container
        m_contDialogue  = new PScrollableComponentContainer(4, PButtonEvent.BID_R1) ;
        m_contDialogue.setBounds(28, 2, 129, 104) ;

        // Init Action ButtonBar
        m_bbActions = new PCommandBar() ;
        m_bbActions.setBounds(0, 0, 160, 108) ;
        m_bbActions.addButton(  new PLabel(getImage("imgBackspaceIcon")),
                                ParameterTextField.ACTION_BACKSPACE,
                                getString("hint/core/edit/backspace")) ;
        m_bbActions.addButton(  new PLabel(getImage("imgCharBackwardIcon")),
                                ParameterTextField.ACTION_BACKWARD,
                                getString("hint/core/edit/movelef")) ;
        m_bbActions.addButton(  new PLabel(getImage("imgCharForwardIcon")),
                                ParameterTextField.ACTION_FORWARD,
                                getString("hint/core/edit/moveright")) ;

        // Disable command bar initially
        m_bbActions.enableByAction(ParameterTextField.ACTION_BACKSPACE, false) ;
        m_bbActions.enableByAction(ParameterTextField.ACTION_BACKWARD, false) ;
        m_bbActions.enableByAction(ParameterTextField.ACTION_FORWARD, false) ;
        m_bbActions.enableByAction(ParameterTextField.ACTION_CLEAR, false) ;

        m_chkDeployment = new PCheckbox(getString("lblUsing"), m_bDeployment) ;
        m_chkDeployment.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        m_chkDeployment.addItemListener(new icCheckboxListener()) ;

        m_chkAsk = new PCheckbox(getString("lblLoginOnReboot"), false) ;
        m_chkAsk.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;

        // Create our text field
        m_txtServer  = new ParameterTextField(m_bbActions) ;
        m_txtServer.setLabel("URL:", PLabel.ALIGN_NORTH |PLabel.ALIGN_WEST) ;
        m_txtServer.setReadOnly(!m_bDeployment) ;

        m_txtServer.setAlphanumericMode(true) ;
        m_txtServer.setFocus(false) ;
        m_txtServer.addFocusListener(new icFocusListener()) ;

        // Set value of checkboxes

        String m_strDeploymentInitial = PingerConfig.getInstance().getValue("PHONESET_DEPLOYMENT_SERVER") ;
        boolean bDeploymentAsk = false ; // Startup prompt set to unchecked unless user has pressed "Later"

        try {
            if (m_strDeploymentInitial.equals("")) {
                m_bDeploymentInitial = false ;
                bDeploymentAsk = true ;
            } else if (m_strDeploymentInitial.equals("-")) {
                m_bDeploymentInitial = false ;
            } else {
                m_bDeploymentInitial = true ;
                m_txtServer.setText(m_strDeploymentInitial) ;
            }
        } catch (NullPointerException e) {
            m_bDeploymentInitial = false ;
            System.out.println("DeploymentPreferencesForm - null pointer exception caught") ;
            SysLog.log(e) ;
            m_txtServer.setText("") ;
        }

        m_bDeployment = m_bDeploymentInitial ;
        m_chkDeployment.setState(m_bDeployment) ;
        m_chkAsk.setState(bDeploymentAsk) ;

        // Add components to scrollable container
        m_contDialogue.addComponent(m_chkDeployment) ;
        m_contDialogue.addComponent(m_txtServer) ;
        m_contDialogue.addComponent(m_chkAsk) ;

        // finally add scrollable container to the device preferences form
        addToDisplayPanel(m_bbActions) ;
        addToDisplayPanel(m_contDialogue) ;

    }


    protected void layoutComponents()
    {
        PContainer container = new PContainer() ;

        container.setLayout(null) ;

        m_bbActions.setBounds(0, 0, 160, 108) ;
        container.add(m_bbActions) ;

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
                "Select the above preferences",
                m_dispatcher,
                m_dispatcher.ACTION_SET) ;
        menuControl.setItem(PBottomButtonBar.B3, itemOK) ;
        menuComponent.addItem(itemOK) ;


        PActionItem itemCancel = new PActionItem(
                new PLabel("Cancel"),
                "Exit the application",
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
     *
     */
    protected void onSet()
    {
        // Check parameters are in correct format
        boolean bFormError = false;
        int     iReturnCode = 0 ;
        String  strErrorMsg = "" ;

        if (m_bReadOnly)
            return ;

        if ((m_chkDeployment.getState() == false)) {
            // Display login form next time reboots
            PingerConfigFileParser.setValue(PingerConfig.PINGER_CONFIG, "PHONESET_DEPLOYMENT_SERVER", "-") ;
        }

        if (m_chkAsk.getState() == true) {
            // Display login form next time reboots
            PingerConfigFileParser.setValue(PingerConfig.PINGER_CONFIG, "PHONESET_DEPLOYMENT_SERVER", "") ;
        } else if (m_chkDeployment.getState() == true ) {
            if (m_txtServer.getText().equals("")) {
                PingerConfigFileParser.setValue(PingerConfig.PINGER_CONFIG, "PHONESET_DEPLOYMENT_SERVER", "-") ;
            } else {
                PingerConfigFileParser.setValue(PingerConfig.PINGER_CONFIG, "PHONESET_DEPLOYMENT_SERVER", m_txtServer.getText()) ;
            }
        }

        closeForm(DIAL) ;

    }

    protected void displayErrorForm (String strErrorMsg)
    {
        SimpleTextForm frmError = new SimpleTextForm(this, "Device Settings Error" );

        strErrorMsg = ""+ strErrorMsg ;

        frmError.setText(strErrorMsg) ;

        frmError.showModal() ;
    }





    /**
     *
     */
    protected void onCancel()
    {
        closeForm(CANCEL) ;
    }


    /**
     *
     */
    protected void onShift()
    {
        //m_tfURL.setShiftLock(!m_tfURL.getShiftLock()) ;
    }


    /**
     *
     */
    protected void onSelect()
    {
        if (m_bDeployment == true) {
            m_txtServer.setReadOnly(false) ;
        } else {
            m_txtServer.setReadOnly(true) ;
        }

    }

    /**
     * @deprecated Hide from users
     */
    public void doLayout()
    {
        layoutComponents() ;
        super.doLayout() ;
    }





   /**
     *
     */
    public class icCommandDispatcher implements PActionListener
    {
        public final String ACTION_SHIFT     = "action_shift" ;
        public final String ACTION_CANCEL    = "action_cancel" ;
        public final String ACTION_SET       = "action_set" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_SHIFT)) {
                onShift() ;
            }
            else if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            }
            else if (event.getActionCommand().equals(ACTION_SET)) {
                onSet() ;
            }

        }
    }


    public class icCheckboxListener implements PItemListener
    {
        public void itemStateChanged(PItemEvent event)
        {
            if (event.getSource() == m_chkDeployment) {
                m_txtServer.setReadOnly(event.getStateChange() != PItemEvent.SELECTED) ;
            }
        }
    }


    public class icFocusListener implements PFocusListener
    {

        public void focusGained(PFocusEvent e)
        {
            m_bbActions.enableByAction(ParameterTextField.ACTION_BACKSPACE, true) ;
            m_bbActions.enableByAction(ParameterTextField.ACTION_BACKWARD, true) ;
            m_bbActions.enableByAction(ParameterTextField.ACTION_FORWARD, true) ;
            m_bbActions.enableByAction(ParameterTextField.ACTION_CLEAR, true) ;
        }


        public void focusLost(PFocusEvent e)
        {
            m_bbActions.enableByAction(ParameterTextField.ACTION_BACKSPACE, false) ;
            m_bbActions.enableByAction(ParameterTextField.ACTION_BACKWARD, false) ;
            m_bbActions.enableByAction(ParameterTextField.ACTION_FORWARD, false) ;
            m_bbActions.enableByAction(ParameterTextField.ACTION_CLEAR, false) ;
        }



    }



}
