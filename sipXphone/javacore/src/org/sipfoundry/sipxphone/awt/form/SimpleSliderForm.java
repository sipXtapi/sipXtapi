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


package org.sipfoundry.sipxphone.awt.form ;

import java.awt.* ;
import java.awt.event.* ;
import java.util.* ;
import java.io.* ;

import java.text.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.app.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.app.adjustvolume.* ;


/**
 * This form contains a simple "slider" implementation; that is, a horizontal
 * sliding bar that reflects changes to a particular value when a user
 * increases or decreases it.
 *
 * @author Andrew Smith
 */
public class SimpleSliderForm extends PApplicationForm
{
    /** Constant value returned from <i>showModal</i> if the "Ok" button is pressed. */
    public static final int OK = 1 ;
    /** Constant value returned from <i>showModal</i> if the "Cancel" button is pressed. */
    public static final int CANCEL = 0 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** Slider component */
    protected PSlider m_slider ;
    /** left-hand Action Button bar */
    protected PCommandBar m_bbActions ;
    /** dispatches commands to various onXXX handlers */
    protected icCommandDispatcher m_commandDispatcher = new icCommandDispatcher() ;

    protected PLabel m_lblInstructions ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Creates a Simple Slider Form with the specified application context and
     * a specified title.
     *
     * @param application The application context for this form.
     * @param strTitle The string to display as the form's title.
     */
    public SimpleSliderForm(Application application, String strTitle)
    {
        super(application, strTitle) ;

        setTitle(strTitle) ;

        // Creates all of our components
        initControls() ;
        initializeCommandbar() ;
        initMenus() ;

        // physically lay everything out
        layoutComponents() ;
    }

    /**
     * Creates a Simple Slider Form with the specified parent form and
     * a specified title.
     *
     * @param formParent Parent form responsible for displaying this sub-form.
     * @param strTitle The string to display as the form's title.
     */
    public SimpleSliderForm(PForm formParent, String strTitle)
    {
        super(formParent, strTitle) ;

        setTitle(strTitle) ;

        // Creates all of our components
        initControls() ;
        initializeCommandbar() ;
        initMenus() ;

        // physically lay everything out
        layoutComponents() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * <i>onCancel</i> is called when the user presses the middle (B2) Cancel
     * button. Simply closes the form by invoking <i>closeForm</i>.
     *
     * @see #closeForm
     */
    public void onCancel()
    {
        closeForm(CANCEL) ;
    }


    /**
     * <i>onExit</i> is called when the user presses the right (B3)
     * button and simply closes the form by invoking <i>closeForm</i>.
     *
     * @see #closeForm
     *
     * @deprecated, this should be onOK
     */
    public void onExit()
    {
        onOk() ;
    }


    /**
     * <i>onOk</i> is called when the user presses the right (B3)
     * button and simply closes the form by invoking <i>closeForm</i>.
     *
     * @see #closeForm
     */
    public void onOk()
    {
        closeForm(OK) ;
    }

    /**
     * <i>onUp</i> is called when the user presses the top left (L1)
     * button and increments the slider value by 1.
     */
    public void onUp()
    {
        m_slider.setUp() ;

    }


    /**
     * <i>onDown</i> is called when the user presses the next-to-top left (L2)
     * button and decreases the slider value by 1.
     */
    public void onDown()
    {
        m_slider.setDown() ;
    }


    /**
     * Set the slider position progammatically.
     *
     * @param iPosition the new position
     */
    public void setPosition(int iPosition)
    {
        m_slider.setPosition(iPosition) ;

    }


   /**
    * Set the slider settings for lower bound, upper bound, initial value, and
    * min/max labels.
    *
    * @param strMin The label for the minimum value (iLow).
    * @param strMax The label for the maximum value (iHigh).
    * @param iLow The lowest value or lower bound for the slider.
    * @param iHigh The highest value or upper bound for the slider.
    * @paran iInitial The initial value for the slider.
    */
    public void setSliderValues(String strMin, String strMax, int iLow, int iHigh, int iInitial)
    {
        m_slider.setValues(strMin, strMax, iLow, iHigh, iInitial) ;
    }


    /**
     * Close this form with the specified exit code. This exit code is
     * returned to the form invoker as the return value of <i>showModal</i>.
     *
     * @param iExitCode The exit code of the form.
     *
     * @see #showModal
     */
    public void closeForm(int iExitCode)
    {
        super.closeForm(iExitCode) ;
    }


    /**
     * Set the help text that will be displayed in this form's help tab.
     * The help text will wrap automatically; however, explicit new lines
     * can be inserted by placing a "\n" into text string.
     *
     * @param strHelp The help string that will be displayed in this form's
     *        help tab.
     */
    public void setHelpText(String strHelp)
    {
        super.setHelpText(strHelp) ;
    }


    /**
     * Get a reference to the bottom button bar. The bottom button bar is a
     * menu-like control that is generally filled with navigational commands.
     *
     * @return A reference to the bottom button bar control.
     */
    public PBottomButtonBar getBottomButtonBar()
    {
        return super.getBottomButtonBar() ;
    }


    /**
     * Set the left column of menu items for this form's menu tab. Each
     * form can display two columns of menu tabs. Please see the xDK
     * <u>Guidelines for User Interface Design</u> for recommendations
     * on menu placement and balancing.
     *
     * @param items An array of PActionItems where each PActionItem represents
     *        a single menu item.
     *
     * @see org.sipfoundry.sipxphone.awt.PActionItem
     */
    public void setLeftMenu(PActionItem[] items)
    {
        super.setLeftMenu(items) ;
    }


    /**
     * Set the right column of menu items for this form's menu tab. Each
     * form can display two columns of menu tabs. Please see the xDK
     * <u>Guidelines for User Interface Design</u> for recommendations
     * on menu placement and balancing.
     *
     * @param items An array of PActionItems where each PActionItem represents
     *        a single menu item.
     *
     * @see org.sipfoundry.sipxphone.awt.PActionItem
     */
    public void setRightMenu(PActionItem[] items)
    {
        super.setRightMenu(items) ;
    }


    /**
     * Show the form in a modal mode. The thread invoking this method will
     * be blocked until the form is closed.
     *
     * @return The return value of the form. This is the value passed into
     *        <i>closeForm</i>.
     *
     * @see #closeForm
     * @see #showModeless
     */
    public int showModal()
    {
        return super.showModal() ;

    }


    /**
     * Show the form in a modeless mode. This method is non-blocking and does
     * not wait for the form to close.
     *
     * @return boolean True on success or false on error.
     *
     * @see #showModal
     */
    public boolean showModeless()
    {
        return super.showModeless() ;
    }


    /**
     * The simple slider form allows you to place instructions and hints below
     * the slider control. This method set the hint/instruction text.
     *
     * @param strText the text that will be display in the slider form.
     */
    public void setInstructions(String strText)
    {
       m_lblInstructions.setText(strText) ;
    }



//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     *
     */
    private void initControls()
    {
        m_slider = new PSlider() ;
        m_lblInstructions = new PLabel() ;

       //add global listener
        getBottomButtonBar().addActionListener(m_commandDispatcher);
    }


    /**
     *
     */
    private void initMenus()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        PActionItem actionExit = new PActionItem( new PLabel(getString("lblGenericOk")),
                getString("hint/xdk/simplesliderform/ok"),
                null, //let global listener get it
                m_commandDispatcher.ACTION_EXIT) ;
        PActionItem menuActionExit = new PActionItem( new PLabel(getString("lblGenericOk")),
                getString("hint/xdk/simplesliderform/ok"),
                m_commandDispatcher,
                m_commandDispatcher.ACTION_EXIT) ;
        menuControl.setItem(PBottomButtonBar.B3, actionExit) ;
        getLeftMenuComponent().addItem(menuActionExit) ;

        PActionItem actionCancel = new PActionItem( new PLabel(getString("lblGenericCancel")),
                getString("hint/xdk/simplesliderform/cancel"),
                null, //let global listener get it
                m_commandDispatcher.ACTION_CANCEL) ;
        PActionItem menuActionCancel = new PActionItem( new PLabel(getString("lblGenericCancel")),
                getString("hint/xdk/simplesliderform/cancel"),
                m_commandDispatcher,
                m_commandDispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, actionCancel) ;
        getLeftMenuComponent().addItem(menuActionCancel) ;
    }


    private void initializeCommandbar()
    {
        m_bbActions = new PCommandBar() ;
        m_bbActions.addActionListener(m_commandDispatcher) ;

        m_bbActions.addButton(  new PLabel(getImage("imgVolumeUp")),
                                m_commandDispatcher.ACTION_UP,
                                getString("hint/xdk/simplesliderform/up")) ;
        m_bbActions.addButton(  new PLabel(getImage("imgVolumeDown")),
                                m_commandDispatcher.ACTION_DOWN,
                                getString("hint/xdk/simplesliderform/down")) ;
    }


    protected void layoutComponents()
    {
        PContainer container = new PContainer() ;
        container.setLayout(null) ;

        m_bbActions.setBounds(0, 0, 28, 27*4) ;
        container.add(m_bbActions) ;

        m_slider.setBounds(28, 25, 160-28-4, 55-25) ;
        container.add(m_slider) ;

        m_lblInstructions.setBounds(28, 55, 160-28-4, (27*4)-55-3) ;
        container.add(m_lblInstructions) ;

        addButtonListener(new icButtonListener()) ;


        addToDisplayPanel(container, new Insets(0, 0, 0, 0)) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////
    /**
     *
     */
    protected class icCommandDispatcher implements PActionListener
    {
        public final String ACTION_UP       = "action_up" ;
        public final String ACTION_DOWN     = "action_down" ;
        public final String ACTION_CANCEL   = "action_cancel" ;
        public final String ACTION_EXIT     = "action_exit" ;


        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_UP)) {
                onUp() ;
                event.consume() ;
            }
            else if (event.getActionCommand().equals(ACTION_DOWN)) {
                onDown() ;
                event.consume() ;
            }
            else if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
                event.consume() ;
            }
            else if (event.getActionCommand().equals(ACTION_EXIT)) {
                onOk() ;
                event.consume() ;
            }
        }
    }


    private class icButtonListener implements PButtonListener
    {
        /**
         * The specified button has been pressed down.
         */
        public void buttonDown(PButtonEvent event)
        {
        }


        /**
         * The specified button has been released.
         */
        public void buttonUp(PButtonEvent event)
        {
            switch (event.getButtonID()) {
                case PButtonEvent.BID_VOLUME_UP:
                    onUp() ;
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_VOLUME_DN:
                    onDown() ;
                    event.consume() ;
                    break ;
            }
        }


        /**
         * The specified button is being held down.
         */
        public void buttonRepeat(PButtonEvent event)
        {

        }
    }
}

