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

/**
 *
 */
public class PasswordForm extends PApplicationForm
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
    protected PPasswordField    m_tfURL ;

    /** Action Button Bar */
    protected PCommandBar m_bbActions ;

    /** container that houses the phone number text field and action button bar */
    protected icPasswordContainer m_contDialer ;

    /** dispatches commands to various onXXX handlers */
    protected icCommandDispatcher m_dispatcher = new icCommandDispatcher() ;

    /** list of listeners awaiting dial notificiation */
    protected Vector m_vListeners = null ;

    /** dial string */
    protected String m_strDial = "" ;

    //protected Call  m_call ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * stock constructor requiring an application context
     */
    public PasswordForm(Application application, String strLogin)
    {
        super(application, "Password") ;

        setStringResourcesFile("PasswordForm.properties") ;
        setTitle(getString("lblPasswordTitle")) ;

        initComponents() ;
        initMenubar() ;
        initMenus() ;

        setHelpText(getString("password"), getString("password_title")) ;
        addFormListener(new icFormListener()) ;
    }


    protected void initComponents()
    {
        // Create our text field
        m_tfURL = new PPasswordField() ;

        // Init Action ButtonBar
        m_bbActions = new PCommandBar() ;
        m_bbActions.addActionListener(m_dispatcher) ;

        m_bbActions.addButton(  new PLabel(getImage("imgBackspaceIcon")),
                                m_dispatcher.ACTION_BACKSPACE,
                                "Backspace") ;
        m_bbActions.addButton(  new PLabel(getImage("imgCharBackwardIcon")),
                                m_dispatcher.ACTION_BACKWARD,
                                "Move the cursor one space to the left") ;
        m_bbActions.addButton(  new PLabel(getImage("imgCharForwardIcon")),
                                m_dispatcher.ACTION_FORWARD,
                                "Move the cursor one space to the right") ;

        m_bbActions.addButton(  new PLabel(getImage("imgClearIcon")),
                                m_dispatcher.ACTION_CLEAR,
                                "Clear the entered URL") ;



        // Create our display container
        m_contDialer = new icPasswordContainer(m_tfURL, m_bbActions) ;
        m_contDialer.setBounds(0, 0, 100, 100) ;
        addToDisplayPanel(m_contDialer, new Insets(0, 0, 0, 0)) ;

        addButtonListener(new icButtonListener()) ;
    }


    protected void initMenubar()
    {
        // Initialize Bottom Menu
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        /*menuControl.setItem(    PBottomButtonBar.B1,
                                    new PLabel("Shift"),
                                    m_dispatcher.ACTION_SHIFT,
                                    "Next letter will be in upper case after select shift") ;*/
        menuControl.setItem(    PBottomButtonBar.B2,
                                    new PLabel("Cancel"),
                                    m_dispatcher.ACTION_CANCEL,
                                    "Exit the application") ;
        menuControl.setItem(    PBottomButtonBar.B3,
                                    new PLabel(getString("lblGenericOk")),
                                    m_dispatcher.ACTION_DIAL,
                                    "Enter the password") ;
    }


    /**
     *  Initialize our pingtel task manager menus
     */
    protected void initMenus()
    {
        PActionItem items[] ;

        // Initialize left menus
        items = new PActionItem[2] ;

        items[0] = new PActionItem(new PLabel(getString("lblGenericOk")),
            getString("password/ok"),
            m_dispatcher,
            m_dispatcher.ACTION_DIAL) ;

        items[1] = new PActionItem(new PLabel("Cancel"),
            getString("password/cancel"),
            m_dispatcher,
            m_dispatcher.ACTION_CANCEL) ;

        setLeftMenu(items) ;

        // Initialize Right Menus
        items = new PActionItem[1] ;

        items[0] = new PActionItem(new PLabel("Clear"),
            getString("password/clear"),
            m_dispatcher,
            m_dispatcher.ACTION_CLEAR) ;

        setRightMenu(items) ;
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
     *
     */
    protected void onDial()
    {
        m_strDial = m_tfURL.getText() ;
        closeForm(DIAL) ;
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
    protected void onCancel()
    {
        m_strDial = "" ;
        closeForm(CANCEL) ;
    }


    /**
     *
     */
    protected void onCaretForward()
    {
        m_tfURL.moveCaretForward() ;
    }


    /**
     *
     */
    protected void onCaretBackward()
    {
        m_tfURL.moveCaretBackward() ;
    }


    /**
     *
     */
    protected void onBackspace()
    {
        m_tfURL.backspace() ;
    }


    /**
     *
     */
    protected void onClear()
    {
        m_tfURL.setText("") ;
    }



    protected class icPasswordContainer extends PContainer
    {
        /** label for our URL */
        protected PLabel     m_lblURL ;
        /** reference to our password field */
        protected PPasswordField m_tfURL ;
        /** reference to our action button bar */
        protected PCommandBar m_bbActions ;
        /** reference to our instructions label */
        protected PLabel     m_lblInstructions ;
         /** the number of dial by url tips */
        protected int        m_iNumTips = - 1 ;


        public icPasswordContainer(PPasswordField tfURL, PCommandBar bbActions)
        {
            m_tfURL = tfURL ;
            m_bbActions = bbActions ;
            m_lblURL = new PLabel(this.getString("lblPasswordLabel"), PLabel.ALIGN_WEST) ;
            m_lblInstructions = new PLabel(this.getString("lblDialByURLTip")) ;

            layoutComponents() ;
        }


        protected void layoutComponents()
        {
            this.setLayout(null) ;

            this.add(m_bbActions) ;
            this.add(m_lblURL) ;
            this.add(m_tfURL) ;
            this.add(m_lblInstructions) ;
        }


        public void setBounds(int x, int y, int w, int h)
        {
            super.setBounds(x, y, w, h) ;

            if (m_bbActions != null)
                m_bbActions.setBounds(0, 0, 28, h) ;

            if (m_lblURL != null)
                m_lblURL.setBounds(28, 0, w-28, 27) ;

            if (m_tfURL != null)
                m_tfURL.setBounds(28, 27, w-28, 17) ;

            if (m_lblInstructions != null)
                m_lblInstructions.setBounds(28, 27+17, w-28, h-(27+17+4)) ;

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

    }


   /**
     *
     */
    public class icCommandDispatcher implements PActionListener
    {
        public final String ACTION_BACKSPACE = "action_backspace" ;
        public final String ACTION_FORWARD   = "action_forward" ;
        public final String ACTION_BACKWARD  = "action_backward" ;
        public final String ACTION_CANCEL    = "action_cancel" ;
        public final String ACTION_DIAL      = "action_dial" ;
        public final String ACTION_CLEAR     = "action_clear" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_BACKSPACE)) {
                onBackspace() ;
            }
            else if (event.getActionCommand().equals(ACTION_FORWARD)) {
                onCaretForward() ;
            }
            else if (event.getActionCommand().equals(ACTION_BACKWARD)) {
                onCaretBackward() ;
            }
            else if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            }
            else if (event.getActionCommand().equals(ACTION_DIAL)) {
                onDial() ;
            }
            else if (event.getActionCommand().equals(ACTION_CLEAR)) {
                onClear() ;
            }

        }
    }

    public class icButtonListener implements PButtonListener
    {
        public void buttonUp(PButtonEvent event)
        {

        }

        public void buttonDown(PButtonEvent event)
        {
            //PhoneHelper.getInstance().stopRingingTone(m_call) ;
        }

        public void buttonRepeat(PButtonEvent event)
        {
        }

    }


    public class icFormListener extends PFormListenerAdapter
    {

        public void formOpening(PFormEvent event)
        {
            //m_tfURL.setDialStateMonitor(m_monitor) ;
            onClear() ;
            //initReadyForm() ;
            //m_monitor.addDialStateMonitorListener(m_dispatcher) ;
        }


        public void formClosing(PFormEvent event)
        {
            //m_tfURL.clearAutoDialTimer() ;
            //m_monitor.removeDialStateMonitorListener(m_dispatcher) ;
        }
    }
}
