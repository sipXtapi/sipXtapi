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


package org.sipfoundry.sipxphone.app.dialbyurl ;

import javax.telephony.* ;

import java.awt.* ;
import java.util.* ;
import java.text.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;

import org.sipfoundry.stapi.* ;

import org.sipfoundry.sipxphone.sys.app.core.*;

/**
 *
 */
public class DialByURLForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** action fired to listeners when time to dial */
    public static final String ACTION_DO_DIAL  = "action_do_dial" ;

    /** return codes from Show Modal: Cancel, do do anything */
    public final static int CANCEL  = 0 ;
    /** return codes from Show Modal: Dial */
    public final static int DIAL    = 1 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** Phone Number Text Field */
    protected PTextField    m_tfURL ;

    /** Action Button Bar */
    protected PCommandBar m_bbActions ;
    /** container that houses the phone number text field and action button bar */
    protected icDialerContainer m_contDialer ;
    /** dispatches commands to various onXXX handlers */
    protected icCommandDispatcher m_commandDispatcher = new icCommandDispatcher() ;
    /** list of listeners awaiting dial notificiation */
    protected Vector m_vListeners = null ;
    /** dial string */
    protected String m_strDial = "" ;

    protected TextFieldActionEventAdapter m_adapter ;

    protected PCall  m_call ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * stock constructor requiring an application context
     */
    public DialByURLForm(Application application, PCall call)
    {
        super(application, "Dial by URL") ;
        m_call = call ;

        setStringResourcesFile("DialByURLForm.properties") ;
        setTitle(getString("lblDialByURLTitle")) ;
        setIcon(getImage("imgDialByURLAppIcon")) ;

        initComponents() ;
        initMenubar() ;
        initMenus() ;

        setHelpText(getString("dial_by_url"), getString("dial_by_url_title")) ;
        addFormListener(new icFormListener()) ;

        enableDialAction() ;
    }


    protected void initComponents()
    {
        // Create our text field
        m_tfURL = new PTextField() ;
        m_tfURL.setLabel("", PLabel.ALIGN_WEST) ;
        m_tfURL.addTextListener(new icTextListener()) ;


        // Init Action ButtonBar
        m_bbActions = new PCommandBar() ;

        m_adapter = new TextFieldActionEventAdapter(m_tfURL, m_bbActions, true) ;
        m_adapter.populateCommandbar() ;

        //allow this textfield to enter ip chars only
        m_adapter.setAllowNumericMode(true);

        // Create our display container
        m_contDialer = new icDialerContainer(m_tfURL, m_bbActions) ;
        addToDisplayPanel(m_contDialer, new Insets(0, 0, 0, 0)) ;

        addButtonListener(new icButtonListener()) ;
    }


    protected void initMenubar()
    {
        // Initialize Bottom Menu
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        menuControl.setItem(    PBottomButtonBar.B2,
                                new PLabel("Cancel"),
                                m_commandDispatcher.ACTION_CANCEL,
                                getString("hint/core/system/cancelform")) ;

        menuControl.setItem(    PBottomButtonBar.B3,
                                new PLabel(Shell.getInstance().getDialingStrategy().getAction()),
                                m_commandDispatcher.ACTION_DIAL,
                                Shell.getInstance().getDialingStrategy().getActionHint()) ;

        menuControl.addActionListener(m_commandDispatcher) ;
    }


    /**
     *  Initialize our pingtel task manager menus
     */
    protected void initMenus()
    {
        PActionItem items[] ;
        PMenuComponent rightMenu = getRightMenuComponent() ;

        // Initialize left menus
        items = new PActionItem[2] ;

        items[0] = new PActionItem(new PLabel(Shell.getInstance().getDialingStrategy().getAction()),
            Shell.getInstance().getDialingStrategy().getActionHint(),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_DIAL) ;

        items[1] = new PActionItem(new PLabel("Cancel"),
            getString("hint/core/system/cancelform"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_CANCEL) ;


        setLeftMenu(items) ;

        rightMenu.removeAllItems();
        rightMenu.addItem(new PActionItem( new PLabel("About"),
                                    getString("hint/core/system/aboutform"),
                                    m_commandDispatcher,
                                    m_commandDispatcher.ACTION_ABOUT)) ;

    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * What string should we dial?
     */
    public String getDialString()
    {
        return m_strDial ;
    }


    /**
     * called when the form is gaining focus
     *
     * @param formLosingFocus form that lost focus as a result of
     */
    public void onFocusGained(PForm formLosingFocus)
    {
        m_contDialer.nextTip() ;
    }


    /**
     *
     */
    protected void onDial()
    {
        String tmpStr = m_tfURL.getText();

        //convert to uppercase before we do the comparison.
        tmpStr = tmpStr.toUpperCase();

        //check to see if sip: appears at the start of the string, or
        //<sip: appears anywhere else. If not, then add sip:
        if (!tmpStr.startsWith("SIP:") && tmpStr.indexOf("<SIP:") == -1)
            m_strDial = "sip:" + m_tfURL.getText() ;
        else
            m_strDial = m_tfURL.getText() ;

        closeForm(DIAL) ;
    }


    /**
     *
     */
    protected void onCancel()
    {
        m_strDial = "" ;
        closeForm(CANCEL) ;
    }



    /**
     *
     */
    protected void onClear()
    {
        m_tfURL.setText("sip:") ;
    }



    protected class icDialerContainer extends PContainer
    {
        /** label for our URL */
        protected PLabel     m_lblURL ;
        /** reference to our phone number text field */
        protected PTextField m_tfURL ;
        /** reference to our action button bar */
        protected PCommandBar m_bbActions ;
        /** reference to our instructions label */
        protected PLabel     m_lblInstructions ;
        /** the number of dial by url tips */
        protected int        m_iNumTips = - 1 ;


        public icDialerContainer(PTextField tfURL, PCommandBar bbActions)
        {
            m_tfURL = tfURL ;
            m_bbActions = bbActions ;
            m_lblURL = new PLabel(this.getString("lblURLAddress"), PLabel.ALIGN_WEST) ;
            m_lblInstructions = new PLabel(this.getString("lblDialByURLTip")) ;

            layoutComponents() ;
        }

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
	            String strTip = this.getString("lblDialByURLTip" + (iTip+1)) ;
	            m_lblInstructions.setText(strTip) ;
	        }
	    }



        protected void layoutComponents()
        {
            this.setLayout(null) ;

            this.add(m_bbActions) ;
            this.add(m_lblURL) ;
            this.add(m_tfURL) ;
            this.add(m_lblInstructions) ;
        }



        public void doLayout()
        {
            Dimension dimSize = this.getSize() ;

            if (m_bbActions != null) {
                m_bbActions.setBounds(0, 0, 27, dimSize.height) ;
                m_bbActions.doLayout() ;
            }

            if (m_lblURL != null) {
                m_lblURL.setBounds(27, 0, dimSize.width-30, 27) ;
                m_lblURL.doLayout() ;
            }

            if (m_tfURL != null) {
                m_tfURL.setBounds(27, 27, dimSize.width-30, 27) ;
                m_tfURL.doLayout() ;
            }

            if (m_lblInstructions != null) {
                m_lblInstructions.setBounds(28, 27+27, dimSize.width-30, dimSize.height-(27+27+4)) ;
                m_lblInstructions.doLayout() ;
            }

            super.doLayout() ;
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
        boolean bEnable = ((m_tfURL.getText() != null) && (m_tfURL.getText().length() > 0)) ;
        enableMenusByAction(bEnable, m_commandDispatcher.ACTION_DIAL) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////
    /**
     *
     */
    private class icCommandDispatcher implements PActionListener
    {
        public final String ACTION_ABOUT    = "action_about" ;
        public final String ACTION_CANCEL    = "action_cancel" ;
        public final String ACTION_DIAL      = "action_dial" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_ABOUT)) {
                SystemAboutBox box = new SystemAboutBox(getApplication());
                box.onAbout();
            }
            else if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            }
            else if (event.getActionCommand().equals(ACTION_DIAL)) {

                //remove any leading and trailing spaces
                m_tfURL.setText(m_tfURL.getText().trim());

                String tmpStr = "";

                //search for an "@" because we only want to reject * that comes after it
                if (m_tfURL.getText().indexOf("@") == -1) //no @ found
                    tmpStr = m_tfURL.getText();
                else
                    tmpStr = m_tfURL.getText().substring(m_tfURL.getText().indexOf("@"));

                if (tmpStr.indexOf("*") != -1)
                {
                    MessageBox messageBox = new MessageBox(getApplication(), MessageBox.TYPE_ERROR);
                    messageBox.setTitle("URL FORMAT ERROR");
                    messageBox.setMessage("You cannot start a URL with an asterisk!");

                    messageBox.showModal();
                    messageBox.closeForm(0);
                }
                else
                    onDial() ;
            }
        }
    }


    /**
     *
     */
    private class icButtonListener implements PButtonListener
    {


        public void buttonDown(PButtonEvent event)
        {
            PCall inFocusCall = Shell.getCallManager().getInFocusCall() ;
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
                case PButtonEvent.BID_MUTE:
                case PButtonEvent.BID_POUND:
                    if (inFocusCall != null) {
                        try {
                            inFocusCall.stopTone() ;
                        } catch (PSTAPIException e) {
                            SysLog.log(e) ;
                        }
                    }
                    break ;
            }
        }

        public void buttonUp(PButtonEvent event) { }
        public void buttonRepeat(PButtonEvent event) { }
    }


    /**
     *
     */
    private class icFormListener extends PFormListenerAdapter
    {

        public void formOpening(PFormEvent event)
        {
            onClear() ;
        }

        public void formClosing(PFormEvent event)  { }
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
            enableDialAction() ;
        }

        public void caretPositionChanged(PTextEvent event) { }
    }
}
