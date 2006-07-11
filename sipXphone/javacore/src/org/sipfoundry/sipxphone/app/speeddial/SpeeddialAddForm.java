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

package org.sipfoundry.sipxphone.app.speeddial;

import java.awt.*;

import org.sipfoundry.util.TextUtils;
import org.sipfoundry.util.AppResourceManager;
import org.sipfoundry.sipxphone.awt.*;
import org.sipfoundry.sipxphone.awt.event.*;
import org.sipfoundry.sipxphone.awt.form.*;

import org.sipfoundry.sipxphone.*;
import org.sipfoundry.sipxphone.sys.*;
import org.sipfoundry.sipxphone.sys.util.*;
import org.sipfoundry.sipxphone.sys.startup.*;

import org.sipfoundry.sipxphone.app.preferences.ParameterTextField;

/**
 *
 */
public class SpeeddialAddForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    /** return codes from Show Modal: Cancel, do do anything */
    public final static int CANCEL  = 0;
    /** return codes from Show Modal: OK */
    public final static int OK      = 1;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** container that houses the controls */
    protected PScrollableComponentContainer m_contFields;

    /** The ID of the speed dial number, e.g. "22" */
    protected PTextField    m_speedDialId ;

    /** The string associated with the speed dial label.*/
    protected PTextField    m_speedDialLabel ;

    /** The string associated with the speed dial number.*/
    protected PTextField    m_speedDialString ;

    /** Phone number Radio button */
    protected PCheckbox     m_chkPhoneNumber;

    // These are used to keep track of the two possibilites for the DialString field
    protected String m_strURL = new String();
    protected String m_strPhoneNumber = new String();

    protected boolean m_bIsPhoneNumber = true;

    /** Action Button Bar */
    protected PCommandBar m_bbActions;

    protected PScrollableComponentContainer m_contOptionList;
    protected SpeeddialTextFieldAdapter m_textfieldAdapter;

    /** dispatches commands to various onXXX handlers */
    protected icCommandDispatcher m_dispatcher = new icCommandDispatcher();

    /** calling application */
    protected Application   m_application;

    protected boolean m_bAdding ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * stock constructor requiring an application context
     */
    public SpeeddialAddForm(Application application)
    {
        this(application, "Add Speed Dial", true);
    }

    public SpeeddialAddForm(Application application, String title, boolean adding)
    {
        super(application, title);

        m_application = application;
        m_bAdding = adding ;

        setStringResourcesFile("SpeeddialAddForm.properties");
        setTitle(title);
        setIcon(getImage("imgSpeedDialIcon")) ;

        initComponents();
        layoutComponents();
        initMenubar();
        initMenus();

        if (adding == true)
        {
            setHelpText(getString("add_speed_dial_number"), getString("add_speed_dial_number_title"));
        } else {
            setHelpText(getString("edit_speed_dial_value"), getString("edit_speed_dial_value_title"));
        }
    }


    public void setInitialFocus()
    {
        if (m_speedDialId != null) {
            m_speedDialId.setFocus(true) ;
        }
    }

    protected void initComponents()
    {
        // Create our scrollable container
        m_contOptionList  = new PScrollableComponentContainer(4, PButtonEvent.BID_R1);
        m_contOptionList.setBounds(28, 2, 129, 104);

        // Init Action ButtonBar
        m_bbActions = new PCommandBar();
        m_bbActions.setBounds(0, 0, 160, 108);
        m_textfieldAdapter = new SpeeddialTextFieldAdapter(m_bbActions);

        m_textfieldAdapter.populateCommandbar() ;

        // Create our checkbox

        m_bIsPhoneNumber    = true;
        m_chkPhoneNumber    = new PCheckbox(getString("lblPhoneNumberCheckbox"), m_bIsPhoneNumber);
        m_chkPhoneNumber.setHint(getString("hint/speeddial/main/is_phonenumber")) ;
        m_chkPhoneNumber.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        m_chkPhoneNumber.addItemListener(new icItemListener()) ;

        // Create our text fields
        m_speedDialId     = new PTextField(5) ;
        m_textfieldAdapter.addTextfield(m_speedDialId, false) ;
        m_speedDialId.setAlphanumericMode(false) ;

        m_speedDialLabel  = new PTextField() ;
        m_textfieldAdapter.addTextfield(m_speedDialLabel, true) ;

        m_speedDialString = new PTextField() ;
        m_textfieldAdapter.addTextfield(m_speedDialString, true) ;
        m_speedDialString.addFocusListener(m_textfieldAdapter);

        m_speedDialString.setLabel("", PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST);

        m_speedDialId.setLabel(getString("lblDialId"), PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST);
        m_speedDialLabel.setLabel(getString("lblDialLabel"), PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST);
        m_speedDialString.setLabel(getString("lblDialPhoneNumber"), PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST);

        m_speedDialLabel.setAlphanumericMode(true);

        // Add components to scrollable container
        m_contOptionList.addComponent(m_speedDialId);
        m_contOptionList.addComponent(m_speedDialLabel);
        m_contOptionList.addComponent(m_chkPhoneNumber);
        m_contOptionList.addComponent(m_speedDialString);


        // finally add scrollable container to the device preferences form
        addToDisplayPanel(m_bbActions);
        addToDisplayPanel(m_contOptionList);

        // Set up the state according to m_bIsPhoneNumber
        setCheckbox();
    }

    protected void layoutComponents()
    {
        PContainer container = new PContainer();

        container.setLayout(null);

        m_bbActions.setBounds(0, 0, 160, 108);
        container.add(m_bbActions);

        m_contOptionList.setBounds(28, 2, 129, 104);
        container.add(m_contOptionList);

        addToDisplayPanel(container, new Insets(0, 0, 0, 0));
    }


    protected void initMenubar()
    {
        // Initialize Bottom Menu
        PBottomButtonBar menuControl = getBottomButtonBar();

        menuControl.setItem(    PBottomButtonBar.B2,
                                new PLabel("Cancel"),
                                m_dispatcher.ACTION_CANCEL,
                                getString("hint/core/system/cancelform"));
        menuControl.setItem(    PBottomButtonBar.B3,
                                new PLabel(getString("lblGenericOk")),
                                m_dispatcher.ACTION_OK,
                                (m_bAdding ? getString("hint/speeddial/add/ok") : getString("hint/speeddial/edit/ok")));

        menuControl.addActionListener(m_dispatcher);
    }


    /**
     *  Initialize our pingtel task manager menus
     */
    protected void initMenus()
    {
        PActionItem items[];

        // Initialize left menus
        items = new PActionItem[2];

        items[0] = new PActionItem(new PLabel(getString("lblGenericOk")),
            (m_bAdding ? getString("hint/speeddial/add/ok") : getString("hint/speeddial/edit/ok")),
            m_dispatcher,
            m_dispatcher.ACTION_OK);

        items[1] = new PActionItem(new PLabel("Cancel"),
            getString("hint/core/system/cancelform"),
            m_dispatcher,
            m_dispatcher.ACTION_CANCEL);

        setLeftMenu(items);

        // Initialize Right Menus
        items = new PActionItem[1];

        setRightMenu(items);
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////


    /**
     *
     */
    public void setEntryId(String s)
    {
        m_speedDialId.setText(s);
    }

    /**
     * remove the leading zeros if they exist
     * before returning.
     */
    public String getEntryId()
    {
        String strRet =  m_speedDialId.getText();
        //if( strRet != null )
        //  strRet = TextUtils.removeLeadingZerosifInt(strRet);
        return strRet;
    }

    /**
     *
     */
    public void setEntryLabel(String s)
    {
        m_speedDialLabel.setText(s);
    }

    /**
     *
     */
    public String getEntryLabel()
    {
        return m_speedDialLabel.getText();
    }

    /**
     *
     */
    public void setEntryPhoneNumber(String s)
    {
        if (s == null)
            s = new String();

        m_strPhoneNumber = s;

        updateDialStringWidget();

    }

    /**
     * did the user select phone number or url?
     * if the radio button is checked, returns true
     * else returns false.
     */
    public boolean isPhoneNumber(){
        return m_bIsPhoneNumber;
    }

    /**
     *
     */
    public String getEntryPhoneNumber()
    {
        if (m_bIsPhoneNumber)
            return m_strPhoneNumber;
        else
            return new String();

    }

    /**
     *
     */
    public void setEntryURL(String s)
    {
        if (s == null)
            s = new String();
        m_strURL = s;
        updateDialStringWidget();

    }

    /**
     *
     */
    public String getEntryURL()
    {
        if (m_bIsPhoneNumber) {
            return new String();
        } else {
            return m_strURL;
        }
    }

    /**
     *
     */
    public void setEntryIsURL(boolean isURL)
    {

        if (isURL==true)
            m_bIsPhoneNumber = false ;
        else
            m_bIsPhoneNumber = true ;

        setCheckbox();
        updateDialStringWidget();
    }

    /**
     *
     */
    public boolean getEntryIsURL()
    {
        if (m_bIsPhoneNumber == false)
            return true ;
        else
            return false ;
    }

    /**
     *
     */
    protected void onOk()
    {
        if (m_bIsPhoneNumber)
        {
            m_strURL = new String();
            m_strPhoneNumber = m_speedDialString.getText();
        } else {
            m_strURL = m_speedDialString.getText();
            m_strPhoneNumber = new String();
        }
        closeForm(OK);
    }

    /**
     *
     */
    protected void onCancel()
    {
        closeForm(CANCEL);
    }


   /**
    *
    */
    public class icCommandDispatcher implements PActionListener
    {
        public final String ACTION_CANCEL    = "action_cancel";
        public final String ACTION_OK        = "action_ok";

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel();
            }
            else if (event.getActionCommand().equals(ACTION_OK)) {
                onOk();
            }
        }
    }


	protected void setCheckbox()
	{
        m_chkPhoneNumber.setState(m_bIsPhoneNumber) ;
	}

    protected void onSelectPhoneNumber()
    {
        if (m_bIsPhoneNumber) 
        {
            m_strURL = m_speedDialString.getText();
            m_speedDialString.setText(m_strPhoneNumber);
        } 
        else 
        {
            m_strPhoneNumber = m_speedDialString.getText();
            m_speedDialString.setText(m_strURL);
        }

        m_speedDialString.setFocus(true);
    }

	protected void onSpeedDialInFocus(boolean focusGained)
	{
        AppResourceManager res = AppResourceManager.getInstance() ;
        if (focusGained && m_bIsPhoneNumber)
        {
            // FUTURE: Allow "i" and "p" only from keyboard
            m_speedDialString.setAlphanumericMode(false);

            // hack to enable activate L4 icon, not really caps lock mode
            // m_textfieldAdapter.enableCapslock(true, m_speedDialString);

            Image img = res.getImage("imgPauseWait");
            m_bbActions.updateButtonLabelByAction(
                TextFieldActionEventAdapter.ACTION_CAPSLOCK, new PLabel(img));
            m_bbActions.updateButtonHintByAction(
                TextFieldActionEventAdapter.ACTION_CAPSLOCK, 
                getString("lblPhoneOptionsHint"));
        }
        else
        {
            m_speedDialString.setAlphanumericMode(true);

            // restore normal caps lock icon
            Image img = res.getImage("imgUprCase");
            m_bbActions.updateButtonLabelByAction(
                TextFieldActionEventAdapter.ACTION_CAPSLOCK, new PLabel(img));
            m_bbActions.updateButtonHintByAction(
                TextFieldActionEventAdapter.ACTION_CAPSLOCK, 
                res.getString("hint/core/edit/capslock_toupper"));
        }
    }

	protected void updateDialStringWidget()
	{
        AppResourceManager res = AppResourceManager.getInstance() ;
        if (m_bIsPhoneNumber == true) {
            m_speedDialString.setAlphanumericMode(false);
            m_speedDialString.setText(m_strPhoneNumber);
            m_textfieldAdapter.enableCapslock(true, m_speedDialString) ;

            Image img = res.getImage("imgPauseWait");
            m_bbActions.updateButtonLabelByAction(
                TextFieldActionEventAdapter.ACTION_CAPSLOCK, new PLabel(img));
            m_bbActions.updateButtonHintByAction(
                TextFieldActionEventAdapter.ACTION_CAPSLOCK, 
                getString("lblPhoneOptionsHint"));
        }
        else
        {
            m_speedDialString.setAlphanumericMode(true);
            m_speedDialString.setText(m_strURL);
            m_textfieldAdapter.enableCapslock(true, m_speedDialString) ;

            Image img = res.getImage("imgUprCase");
            m_bbActions.updateButtonLabelByAction(
                TextFieldActionEventAdapter.ACTION_CAPSLOCK, new PLabel(img));
            m_bbActions.updateButtonHintByAction(
                TextFieldActionEventAdapter.ACTION_CAPSLOCK, 
                res.getString("hint/core/edit/capslock_toupper"));
        }

        m_speedDialString.setFocus(true);
	}

    /**
     *
    protected void onSelectPhoneNumber()
    {
        if (m_bIsPhoneNumber == true) 
        {
            m_strURL = m_speedDialString.getText();
        } 
        else 
        {
            m_strPhoneNumber = m_speedDialString.getText();
        }

        updateDialStringWidget();
    }
     */

    protected void onPauseWait()
    {
        PhoneNumberOptions options = new PhoneNumberOptions(m_application);
        if (options.showModal() == 1)
        {
            m_speedDialString.setText(m_speedDialString.getText() + options.getSelectedItem());
        }
    }

    public class icItemListener implements PItemListener
    {

        public void itemStateChanged(PItemEvent event)
        {
            if (event.getStateChange() == PItemEvent.SELECTED)
                m_bIsPhoneNumber = true;
            else
                m_bIsPhoneNumber = false;

            onSelectPhoneNumber();
        }
    }

    class SpeeddialTextFieldAdapter extends TextFieldActionEventAdapter implements PFocusListener
    {
        public SpeeddialTextFieldAdapter(PCommandBar actions)
        {
            super(actions);
        }

        public void onProxyCapslock()
        {
            // pause and wait icon does not toggle
            if (m_tfTextfield == m_speedDialString && m_bIsPhoneNumber)
            {
                onPauseWait();
            }
            else
            {
                super.onProxyCapslock();
            }
        }

        public void focusGained(PFocusEvent e)
        {
            if (e.getSource() == m_speedDialString)
            {
                onSpeedDialInFocus(true);
            }
        }
        
        public void focusLost(PFocusEvent e)
        {
            if (e.getSource() == m_speedDialString)
            {
                onSpeedDialInFocus(false);
            }
        }
    }
}
