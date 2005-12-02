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

import java.awt.Container;
import java.awt.Insets;

import org.sipfoundry.util.TextUtils;
import org.sipfoundry.sipxphone.awt.*;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.Application ;
import org.sipfoundry.sipxphone.sys.SystemDefaults ;
import org.sipfoundry.sipxphone.sys.Shell ;

/**
 *
 */
public class CallHandlingForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    /** return codes from Show Modal: Cancel, do do anything */
    public final static int CANCEL  = 0 ;
    /** return codes from Show Modal: Set */
    public final static int SET    = 1 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** container that houses the controls */
    protected PScrollableComponentContainer m_contOptionList ;

    /** URL to forward calls to on no answer*/
    protected ParameterTextField    m_tForwardBusy ;
    /** URL to forward calls to on busy*/
    protected ParameterTextField    m_tForwardNoAnswer ;
    /** URL to forward all calls to */
    protected ParameterTextField    m_tForwardAll ;

    /** ForwardNoAnswerTimeout choice control */
    protected PChoice m_choiceForwardNoAnswerTimeout;

    /** Forward on busy checkbox */
    protected PCheckbox     m_chkForwardBusy ;
    /** Forward on no answer checkbox*/
    protected PCheckbox     m_chkForwardNoAnswer ;
    /** Forward on all checkbox */
    protected PCheckbox     m_chkForwardAll ;
    /** Use Call Waiting checkbox */
    protected PCheckbox     m_chkCallWaiting ;

    protected PCheckboxGroup m_grpCallIndication = new PCheckboxGroup() ;

    /** Do not disturb checkbox */
    protected PCheckbox     m_chkDoNotDisturb ;
    /** Ringer visual checkbox */
    protected PCheckbox     m_chkCallIndicationVisual ;
    /** Ringer audio checkbox */
    protected PCheckbox     m_chkCallIndicationAuditory ;
    /** Ringer audio checkbox */
    protected PCheckbox     m_chkCallIndicationBoth ;


    /** Action Button Bar */
    protected PCommandBar m_bbActions ;

    /** dispatches commands to various onXXX handlers */
    protected icCommandDispatcher m_dispatcher = new icCommandDispatcher() ;

    /** Call Handling Manager - responsible for getting/setting values */
    protected CallHandlingManager m_callManager ;

    //text field adapter
    protected TextFieldActionEventAdapter m_adapter ;

    /** the application associated with this form */
    protected Application m_application;
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * stock constructor requiring a parent form
     */
    public CallHandlingForm(PForm formParent)
    {
        super(formParent, "Call Handling Preferences");
        setStringResourcesFile("CallHandlingForm.properties") ;
        setTitle(getString("lblCallHandlingTitle")) ;
        setIcon(getImage("imgPrefsAppsIcon")) ;

        initComponents() ;
        layoutComponents() ;
        initMenubar() ;
        setHelpText(getString("preference_call"), getString("preference_call_title")) ;
    }


    protected void initComponents()
    {
        // Create our scrollable container
        m_contOptionList  = new PScrollableComponentContainer(4, PButtonEvent.BID_R1) ;
        m_contOptionList.setBounds(28, 2, 129, 104) ;

        // Init Action ButtonBar
        m_bbActions = new PCommandBar() ;
        m_bbActions.setBounds(0, 0, 160, 108) ;
        m_bbActions.addButton(  new PLabel(getImage("imgBackspaceIcon")),
                                ParameterTextField.ACTION_BACKSPACE,
                                getString("hint/core/edit/backspace")) ;
        m_bbActions.addButton(  new PLabel(getImage("imgCharBackwardIcon")),
                                ParameterTextField.ACTION_BACKWARD,
                                getString("hint/core/edit/moveleft")) ;
        m_bbActions.addButton(  new PLabel(getImage("imgCharForwardIcon")),
                                ParameterTextField.ACTION_FORWARD,
                                getString("hint/core/edit/moveright")) ;


        // Disable command bar initially
        m_bbActions.enableByAction(ParameterTextField.ACTION_BACKSPACE, false) ;
        m_bbActions.enableByAction(ParameterTextField.ACTION_BACKWARD, false) ;
        m_bbActions.enableByAction(ParameterTextField.ACTION_FORWARD, false) ;
        m_bbActions.enableByAction(ParameterTextField.ACTION_CLEAR, false) ;

        // Get Call Handling Info
        m_callManager = Shell.getInstance().getCallHandlingManager();

        // Listener for all check boxes
        icCheckboxListener checkboxListener = new icCheckboxListener() ;

        // Create our check boxes
        m_chkDoNotDisturb = new PCheckbox(getString("lblDoNotDisturb")) ;
        m_chkDoNotDisturb.setHint(getString("hint/preference/call_handling/do_not_disturb")) ;
        m_chkDoNotDisturb.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        m_chkDoNotDisturb.setState(m_callManager.isDNDEnabled()) ;
        m_chkDoNotDisturb.addItemListener(checkboxListener) ;

        m_chkForwardAll = new PCheckbox(getString("lblForwardAll")) ;
        m_chkForwardAll.setHint(getString("hint/preference/call_handling/forward_all_calls")) ;
        m_chkForwardAll.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        m_chkForwardAll.addItemListener(checkboxListener) ;

        m_chkForwardNoAnswer = new PCheckbox(getString("lblForwardNoAnswer")) ;
        m_chkForwardNoAnswer.setHint(getString("hint/preference/call_handling/forward_no_answer")) ;
        m_chkForwardNoAnswer.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        m_chkForwardNoAnswer.addItemListener(checkboxListener) ;

        m_chkCallWaiting = new PCheckbox(getString("lblUseCallWaiting")) ;
        m_chkCallWaiting.setHint(getString("hint/preference/call_handling/use_call_waiting")) ;
        m_chkCallWaiting.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        m_chkCallWaiting.addItemListener(checkboxListener) ;

        m_chkForwardBusy = new PCheckbox(getString("lblForwardBusy")) ;
        m_chkForwardBusy.setHint(getString("hint/preference/call_handling/forward_busy")) ;
        m_chkForwardBusy.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        m_chkForwardBusy.addItemListener(checkboxListener) ;

        m_chkCallIndicationVisual = new PCheckbox(getString("lblCallIndicationVisual")) ;
        m_chkCallIndicationVisual.setHint(getString("hint/preference/call_handling/visual_indication")) ;
        m_chkCallIndicationVisual.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        m_chkCallIndicationVisual.setCheckboxGroup(m_grpCallIndication) ;

        m_chkCallIndicationAuditory = new PCheckbox(getString("lblCallIndicationAuditory")) ;
        m_chkCallIndicationAuditory.setHint(getString("hint/preference/call_handling/audible_indication")) ;
        m_chkCallIndicationAuditory.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        m_chkCallIndicationAuditory.setCheckboxGroup(m_grpCallIndication) ;

        m_chkCallIndicationBoth = new PCheckbox(getString("lblCallIndicationBoth")) ;
        m_chkCallIndicationBoth.setHint(getString("hint/preference/call_handling/visual_audible_indication")) ;
        m_chkCallIndicationBoth.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        m_chkCallIndicationBoth.setCheckboxGroup(m_grpCallIndication) ;

        // Create our text fields
        m_tForwardBusy      = new ParameterTextField(m_bbActions) ;
        m_tForwardBusy.setLabel(getString("lblForwardTo"), PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST) ;
        m_tForwardNoAnswer  = new ParameterTextField(m_bbActions) ;
        m_tForwardNoAnswer.setLabel(getString("lblForwardTo"), PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST) ;
        m_tForwardAll       = new ParameterTextField(m_bbActions) ;
        m_tForwardAll.setLabel(getString("lblForwardTo"), PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST) ;

       // forward no answer timeout choice control.
        m_choiceForwardNoAnswerTimeout =
            new PChoice(this, getString("lblForwardAfterHint"),
                        getString("lblForwardAfterTitle"));
        m_choiceForwardNoAnswerTimeout.setLabel(getString("lblForwardAfter"), PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST ) ;
        m_choiceForwardNoAnswerTimeout.setHelpText(getString("helpPhonesetNoAnswerTimeout"));
        int iPhonesetNoAnswerTimeout = m_callManager.getPhonesetNoAnswerTimeout();
        int iIncrement = 6;
        boolean bSelected = false;
        for( int i = 1; i<6; i++ ){
            int iValue = iIncrement*i;
            StringBuffer bufferDisplayValue = new StringBuffer();
            bufferDisplayValue.append(String.valueOf(iValue));
            bufferDisplayValue.append(" seconds (");
            bufferDisplayValue.append(String.valueOf(i));
            bufferDisplayValue.append(" ring");
            if( i > 1 )
                bufferDisplayValue.append("s");
            bufferDisplayValue.append(")");

            m_choiceForwardNoAnswerTimeout
                .addItem(new icTimeDisplay(iValue, bufferDisplayValue.toString()));
            if( iPhonesetNoAnswerTimeout == iValue ){
               m_choiceForwardNoAnswerTimeout.setSelectedIndex(i-1);
               bSelected = true;
            }
            //to be compatible with values entered with multiples of 4
            //which we no longer allow
            else if ( (iPhonesetNoAnswerTimeout < (iValue + iIncrement) ) && (!bSelected)){
               m_choiceForwardNoAnswerTimeout.setSelectedIndex(i-1);
           }

        }


        m_adapter = new TextFieldActionEventAdapter(m_tForwardNoAnswer, m_bbActions, true) ;
        m_adapter.addTextfield(m_tForwardBusy, true);
        m_adapter.addTextfield(m_tForwardAll,true);
        m_bbActions.disable();
        m_adapter.populateCommandbar() ;
        m_adapter.enableCommandbar(false);

        // Set textfields to alphanumeric
        m_tForwardBusy.setAlphanumericMode(true) ;
        m_tForwardNoAnswer.setAlphanumericMode(true) ;
        m_tForwardAll.setAlphanumericMode(true) ;

        // Create Focus listener for all text fields
        icFocusListener focusListener = new icFocusListener() ;

        // Add focus listeners - these will remove command bar when text field is not in focus
        m_tForwardBusy.addFocusListener(focusListener) ;
        m_tForwardNoAnswer.addFocusListener(focusListener) ;
        m_tForwardAll.addFocusListener(focusListener) ;

        // Set values of text fields
        m_tForwardAll.setText(m_callManager.getSipForwardingAll()) ;
        m_tForwardNoAnswer.setText(m_callManager.getSipForwardingNoAnswer()) ;
        m_tForwardBusy.setText(m_callManager.getSipForwardingBusy()) ;

        // Add components to scrollable container
        m_contOptionList.addComponent(m_chkDoNotDisturb) ;

        m_contOptionList.addComponent(m_chkForwardAll) ;
        m_chkForwardAll.setState(m_callManager.isForwardingAllEnabled()) ;

        m_contOptionList.addComponent(m_chkForwardNoAnswer) ;
        m_chkForwardNoAnswer.setState(m_callManager.isForwardOnNoAnswerEnabled()) ;

        m_contOptionList.addComponent(m_chkCallWaiting) ;

        m_contOptionList.addComponent(m_chkForwardBusy) ;
        m_chkForwardBusy.setState(m_callManager.isForwardOnBusyEnabled()) ;

        m_contOptionList.addComponent(m_chkCallIndicationVisual) ;
        m_contOptionList.addComponent(m_chkCallIndicationAuditory) ;
        m_contOptionList.addComponent(m_chkCallIndicationBoth) ;


        switch (m_callManager.getCallIndication()) {
            case CallHandlingManager.RINGER_AUDIBLE:
                m_grpCallIndication.setSelectedCheckbox(m_chkCallIndicationAuditory) ;
                break ;
            case CallHandlingManager.RINGER_VISUAL:
                m_grpCallIndication.setSelectedCheckbox(m_chkCallIndicationVisual) ;
                break ;
            default:
                m_grpCallIndication.setSelectedCheckbox(m_chkCallIndicationBoth) ;
                break ;
        }

        m_chkCallWaiting.setState(m_callManager.isCallWaitingEnabled()) ;

        // finally add scrollable container to the device preferences form
        addToDisplayPanel(m_bbActions) ;
        addToDisplayPanel(m_contOptionList) ;
    }


    protected void layoutComponents()
    {
        PContainer container = new PContainer() ;

        container.setLayout(null) ;

        m_bbActions.setBounds(0, 0, 160, 108) ;
        container.add(m_bbActions) ;

        m_contOptionList.setBounds(28, 2, 129, 104) ;
        container.add(m_contOptionList) ;

        addToDisplayPanel(container, new Insets(0, 0, 0, 0)) ;
    }


    protected void initMenubar()
    {
        // Initialize Bottom Menu
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        PActionItem itemSet = new PActionItem(
                new PLabel(getString("lblGenericOk")),
                getString("hint/preferences/common/ok"),
                m_dispatcher,
                m_dispatcher.ACTION_SET) ;

        menuControl.setItem(PBottomButtonBar.B3, itemSet) ;
        getLeftMenuComponent().addItem(itemSet) ;

        PActionItem itemCancel = new PActionItem(
                new PLabel("Cancel"),
                getString("hint/preferences/common/cancel"),
                m_dispatcher,
                m_dispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, itemCancel) ;
        getLeftMenuComponent().addItem(itemCancel) ;
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
        String strErrorMessage = validateEntries();
        if( strErrorMessage.length() > 0 )
            displayErrorForm( strErrorMessage );
        else{
            m_callManager.setSipForwardingAll(m_tForwardAll.getText());
            m_callManager.setPhonesetForwardingAll( m_chkForwardAll.getState());

            m_callManager.setSipForwardingBusy(m_tForwardBusy.getText()) ;
            if (m_chkForwardBusy.getState() == true) {
                m_callManager.setPhonesetBusyBehavior(CallHandlingManager.FORWARD);
            } else {
                m_callManager.setPhonesetBusyBehavior(CallHandlingManager.BUSY) ;
            }


            m_callManager.setSipForwardingNoAnswer
                (m_tForwardNoAnswer.getText(),
                    ((icTimeDisplay)(m_choiceForwardNoAnswerTimeout.getSelectedItem())).getTime()) ;

            if (m_chkForwardNoAnswer.getState() == true) {
                m_callManager.setPhonesetAvailableBehavior
                        (CallHandlingManager.FORWARD_ON_NO_ANSWER);
            }else{
                m_callManager.setPhonesetAvailableBehavior(CallHandlingManager.RING) ;
            }

            /** Save parameters */



            m_callManager.setCallWaiting(m_chkCallWaiting.getState()) ;

            if (m_chkCallIndicationVisual.getState()) {
                m_callManager.setCallIndication(m_callManager.RINGER_VISUAL) ;
            } else if (m_chkCallIndicationAuditory.getState()) {
                m_callManager.setCallIndication(m_callManager.RINGER_AUDIBLE) ;
            } else {
                m_callManager.setCallIndication(m_callManager.RINGER_BOTH) ;
            }

            m_callManager.enableDND(m_chkDoNotDisturb.getState()) ;

            // Adjust the DND feature indicator as needed.
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


            closeForm(SET) ;
        }
    }

    /**
     * validates the call hadnling settings entries.
     * The validation is limited to checking if the sip url provided are null.
     */
    protected String  validateEntries(){
        StringBuffer  strErrorMsgBuffer = new StringBuffer() ;
        if ( (m_chkForwardAll.getState() == true) &&
             (TextUtils.isNullOrSpace(m_tForwardAll.getText())) ) {
             strErrorMsgBuffer.append("Address for forwarding calls when busy should be provided.\n");
        }

        if ( (m_chkForwardBusy.getState() == true) &&
             (TextUtils.isNullOrSpace(m_tForwardBusy.getText()) ) ) {
             strErrorMsgBuffer.append("Address for forwarding calls when busy should be provided.\n");
        }

        if ( (m_chkForwardNoAnswer.getState() == true) &&
             (TextUtils.isNullOrSpace(m_tForwardNoAnswer.getText()) ) ) {
             strErrorMsgBuffer.append("Address for forwarding calls when not answered should be provided.\n");
        }
        return strErrorMsgBuffer.toString();

    }

   /**
    * Displays the error form.
    */
    protected void displayErrorForm (String strErrorMsg)
    {

        SimpleTextForm frmError = new SimpleTextForm(CallHandlingForm.this, "Call Handling Error" );

        strErrorMsg = getString("strFormErrorMessage") + strErrorMsg ;

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
    public class icCommandDispatcher implements PActionListener
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

    public class icCheckboxListener implements PItemListener
    {
        public void itemStateChanged(PItemEvent event)
        {
            if (event.getStateChange() == PItemEvent.SELECTED) {

                if (event.getSource() == m_chkForwardAll) {
                    m_contOptionList.insertComponentAfter(m_tForwardAll, m_chkForwardAll) ;
                    m_tForwardAll.setFocus(true) ;
                } else if (event.getSource() == m_chkForwardBusy) {
                    m_contOptionList.insertComponentAfter(m_tForwardBusy, m_chkForwardBusy) ;
                    m_tForwardBusy.setFocus(true) ;
                } else if (event.getSource() == m_chkForwardNoAnswer) {
                    m_contOptionList.insertComponentAfter(m_tForwardNoAnswer, m_chkForwardNoAnswer) ;
                    m_contOptionList.insertComponentAfter(m_choiceForwardNoAnswerTimeout, m_tForwardNoAnswer);
                    m_tForwardNoAnswer.setFocus(true) ;
                } else if (event.getSource() == m_chkDoNotDisturb) {
                    MessageBox msgBox = new MessageBox(CallHandlingForm.this, MessageBox.TYPE_WARNING) ;
                    msgBox.setMessage(getString("lblWarningDoNotDisturb")) ;
                    if (msgBox.showModal() != MessageBox.OK) {
                        m_chkDoNotDisturb.setState(false) ;
                    }
                }
            } else if (event.getStateChange() == PItemEvent.DESELECTED) {
                if (event.getSource() == m_chkForwardAll) {
                    m_contOptionList.removeComponent(m_tForwardAll) ;
                } else if (event.getSource() == m_chkForwardBusy) {
                    m_contOptionList.removeComponent(m_tForwardBusy) ;
                } else if (event.getSource() == m_chkForwardNoAnswer) {
                    m_contOptionList.removeComponent(m_tForwardNoAnswer) ;
                    m_contOptionList.removeComponent(m_choiceForwardNoAnswerTimeout) ;
                }
            }
        }
    }



    public class icFocusListener implements PFocusListener
    {

        public void focusGained(PFocusEvent e)
        {
            m_bbActions.enable();
            m_adapter.enableCommandbar(true);
//            m_bbActions.enableByAction(ParameterTextField.ACTION_BACKSPACE, true) ;
//            m_bbActions.enableByAction(ParameterTextField.ACTION_BACKWARD, true) ;
//            m_bbActions.enableByAction(ParameterTextField.ACTION_FORWARD, true) ;
//            m_bbActions.enableByAction(ParameterTextField.ACTION_CLEAR, true) ;

        }


        public void focusLost(PFocusEvent e)
        {
            m_bbActions.disable();
            m_adapter.enableCommandbar(false);
//            m_bbActions.enableByAction(ParameterTextField.ACTION_BACKSPACE, false) ;
//            m_bbActions.enableByAction(ParameterTextField.ACTION_BACKWARD, false) ;
//            m_bbActions.enableByAction(ParameterTextField.ACTION_FORWARD, false) ;
//            m_bbActions.enableByAction(ParameterTextField.ACTION_CLEAR, false) ;

        }
    }

    /** private class to encapsulate an int and  a string
     *  which is useful if you have a int as a value but
     *  you want to show a different string as a display value.
     */
    private class icTimeDisplay{
        /** int value of this object */
        private int m_iTime;

        /** display value as a string of this object */
        private String m_strDisplay;

        /** takes an int and a string */
        public icTimeDisplay( int iTime, String strDisplay ){
           m_iTime = iTime;
           m_strDisplay = strDisplay;
        }
        /** get Time as an int. */
        public int getTime(){
          return m_iTime;
        }
       /** get Display value for the time
        *  like "2 seconds" for the time value of "2".
        */
        public String getDisplay(){
          return m_strDisplay;
        }

       /** This returns you the display string
        *  which is useful when showing in a list.
        */
        public String toString(){
          return m_strDisplay;
        }

    }
}
