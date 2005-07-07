/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/awt/form/ComplexForm.java#2 $
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

import java.util.* ;
import java.awt.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;


/**
 * <<TODO:: DESCRIBE ME>>
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ComplexForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Constant value returned from <i>showModal</i> if the "Ok" button is pressed. */
    public static final int OK = 1 ;
    /** Constant value returned from <i>showModal</i> if the "Cancel" button is pressed. */
    public static final int CANCEL = 0 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private PScrollableComponentContainer m_container ;
    private icActionDispatcher m_actionDispatcher ;

    private icDisplayContainer m_contDisplay ;

    private TextFieldActionEventAdapter m_textfieldAdapter ;
    private PCommandBar m_commandBar ;

    private boolean m_bCommandBarEnabled ;



//////////////////////////////////////////////////////////////////////////////
// Constructors
////
    /**
     * Creates a complex form within the specified application context
     * and with the specified form title.
     *
     * @param application Reference to the invoking application.
     * @param strTitle The title of the form (to display in the title bar).
     */
    public ComplexForm(Application application,
                            String      strTitle)
    {
        super(application, strTitle) ;
        setTitle(strTitle) ;

        m_actionDispatcher = new icActionDispatcher() ;
        m_bCommandBarEnabled = false ;

        // Creates all of our components.
        initControls() ;
        initMenus() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Handler invoked when the end user presses the "Ok" button. The default
     * action closes the form with a status of "SimpleOptionForm.OK".
     */
    public void onOk()
    {
        closeForm(OK) ;
    }


    /**
     * Handler invoked when the end user presses the "Cancel" button. The
     * default action closes the form with a status of "SimpleOptionForm.CANCEL".
     */
    public void onCancel()
    {
        closeForm(CANCEL) ;
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
     * Set the help text that will display on this form's help tab.
     * The help text will wrap automatically; however, explicit new lines
     * can be inserted by placing a "\n" into the text string.
     *
     * @param strHelp The help string that will be displayed on this form's
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
     * not wait form the form to close.
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
     * Adds a component to this complex form.  It is assumed that the component
     * will take a single line.
     */
    public void addComponent(PComponent component)
    {
        if (component instanceof PTextField)
        {
            PTextField textField = (PTextField) component ;
            m_textfieldAdapter.addTextfield(textField, textField.getAlphanumericMode()) ;
        }

        m_container.addComponent(component) ;
    }


    /**
     * Updates a component
     */
    public void updateComponent(PComponent component)
    {
        if (component instanceof PTextField)
        {
            PTextField textField = (PTextField) component ;
            m_textfieldAdapter.enableCapslock(textField.getAlphanumericMode(), textField) ;
        }
    }


    /**
     * Removes a component from this complex form.
     */
    public void removeComponent(PComponent component)
    {
        m_container.removeComponent(component) ;

        if (component instanceof PTextField)
        {

        }


    }


    /**
     * Is the command bar enabled and currently visible in this form?
     * The default is false and developers need to explicitly turn this
     * on by invoking <i>enableCommandBar</i>. When the command bar is enabled,
     * it presents additional actions to users.
     *
     * @return True if the command bar is enabled, otherwise false.
     *
     * @see #enableCommandBar
     * @see #getCommandBar
     */
    public boolean isCommandBarEnabled()
    {
        return (m_bCommandBarEnabled) ;
    }


    /**
     * Enable or disable the command bar.
     *
     * @param bEnable True to enable and show the command bar, or false to disable
     *        and hide it.
     *
     * @see #isCommandBarEnabled
     * @see #getCommandBar
     */
    public void enableCommandBar(boolean bEnable)
    {
        m_bCommandBarEnabled = true ;
        doLayout() ;
    }


    /**
     * Get a reference to the command bar. By default, the command bar is
     * disabled and developers must call <i>enableCommandBar(true)</i> prior to
     * invoking this accessor method.
     *
     * @return Reference to the command bar, or null if not enabled.
     *
     * @see #isCommandBarEnabled
     * @see #enableCommandBar
     */
    public PCommandBar getCommandBar()
    {
        return m_commandBar ;
    }


    /**
     * @deprecated do not expose
     */
    public void doLayout()
    {
        if (m_contDisplay != null)
            m_contDisplay.doLayout() ;

        super.doLayout() ;
    }




//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Initialize the bottom button bar with "Ok" and "Cancel" buttons.
     */
    private void initMenus()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        PActionItem actionOk = new PActionItem( new PLabel(getString("lblGenericOk")),
                getString("hint/xdk/simpleoptionform/ok"),
                m_actionDispatcher,
                m_actionDispatcher.ACTION_OK) ;
        menuControl.setItem(PBottomButtonBar.B3, actionOk) ;
        getLeftMenuComponent().addItem(actionOk) ;

        PActionItem actionCancel = new PActionItem( new PLabel(getString("lblGenericCancel")),
                getString("hint/xdk/simpleoptionform/cancel"),
                m_actionDispatcher,
                m_actionDispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, actionCancel) ;
        getLeftMenuComponent().addItem(actionCancel) ;
    }


    /**
     * Initializes the container and adds it to the form.
     */
    private void initControls()
    {
        m_container = new PScrollableComponentContainer(4, PButtonEvent.BID_R1) ;

        m_commandBar = new PCommandBar() ;
        m_textfieldAdapter = new TextFieldActionEventAdapter(m_commandBar) ;
        m_textfieldAdapter.populateCommandbar() ;

        if (m_contDisplay == null) {
            m_contDisplay = new icDisplayContainer() ;
            m_contDisplay.setLayout(null) ;
            m_contDisplay.add(m_container) ;
        }

        addToDisplayPanel(m_contDisplay, new Insets(1,1,1,1)) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////
    /**
     * Helper class that listens for action commands and dispatches to various
     * onXXX handlers.
     */
    private class icActionDispatcher implements PActionListener
    {
        public final String ACTION_CANCEL = "ACTION_CANCEL" ;
        public final String ACTION_OK = "ACTION_OK" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_OK))
                onOk() ;
            else if (event.getActionCommand().equals(ACTION_CANCEL))
                onCancel() ;
        }
    }


    private class icDisplayContainer extends PContainer
    {
        public icDisplayContainer()
        {
            this.setOpaque(false) ;
        }

        public void doLayout()
        {
            Dimension dim = this.getSize() ;

            if (m_bCommandBarEnabled)
            {
                if (! this.containsComponent(m_commandBar))
                    this.add(m_commandBar) ;

                m_commandBar.setBounds(0, 0, 28, dim.height) ;
                m_container.setBounds(28, 0, dim.width-30, dim.height) ;
            }
            else
            {
                if ( this.containsComponent(m_commandBar))
                    this.remove(m_commandBar) ;

                m_container.setBounds(0, 0, dim.width, dim.height) ;
            }

            super.doLayout() ;
        }
    }
}
