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

import java.util.* ;
import java.awt.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;


/**
 * SimpleOptionForm allows you to present a list of selectable
 * options to the end user. The selectable options are presented as a list
 * of independent check boxes or as a group of related check boxes (radio buttons).
 * By default, all options are ungrouped. Invoke the <i>groupOptions</i>
 * method to group check boxes.
 * <p>
 * To add options, invoke the <i>addOption</i> method; then
 * get values by invoking the <i>getOption</i> method.
 * <p>
 * The Bottom button bar is initialized with a "Cancel" button at position B2
 * (bottom center) and an "Ok" button at position B3 (bottom right).
 * If the <i>showModal</i> method is
 * used to display this form, the return code is either SimpleOptionForm.OK
 * or SimpleOptionForm.CANCEL depending on which of the bottom buttons was
 * pressed.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SimpleOptionForm extends PApplicationForm
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
    private PScrollableComponentContainer m_checkboxes ;
    private icActionDispatcher m_actionDispatcher ;
    private Hashtable m_htCheckboxes ;
    private PCheckboxGroup m_group ;

//////////////////////////////////////////////////////////////////////////////
// Constructors
////
    /**
     * Creates a simple option form within the specified application context
     * and with the specified form title.
     *
     * @param application Reference to the invoking application.
     * @param strTitle The title of the form (to display in the title bar).
     */
    public SimpleOptionForm(Application application,
                            String      strTitle)
    {
        super(application, strTitle) ;
        setTitle(strTitle) ;

        m_actionDispatcher = new icActionDispatcher() ;
        getBottomButtonBar().addActionListener(m_actionDispatcher);
        m_htCheckboxes = new Hashtable() ;
        m_group = null ;

        // Creates all of our components.
        initControls() ;
        initMenus() ;
    }


    /**
     * Creates a simple option form within the specified parent form
     * and with the specified form title.
     *
     * @param formParent Parent form responsible for displaying this sub-form.
     * @param strTitle The title of the form (to display in the title bar).
     */
    public SimpleOptionForm(PForm  formParent,
                            String strTitle)
    {
        super(formParent, strTitle) ;
        setTitle(strTitle) ;

        m_actionDispatcher = new icActionDispatcher() ;
        getBottomButtonBar().addActionListener(m_actionDispatcher);
        m_htCheckboxes = new Hashtable() ;
        m_group = null ;

        // Creates all of our components.
        initControls() ;
        initMenus() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Group all options into a single logical choice. After grouping,
     * the end user will be able to select exactly one option -- much like a
     * set of Radio Buttons in other graphical user interface implementations.
     *
     * This method must be invoked before any options are added.
     */
    public void groupOptions()
    {
        if (m_group == null)
            m_group = new PCheckboxGroup() ;
    }


    /**
     * Return the 0-based index of the selected group option. The index is
     * calculated by returning the physical order of the option as it was added
     * to this simple option form.
     *
     * @return The index of the selected group option or -1 if no items are selected.
     */
    public int getSelectedGroupOptionIndex()
    {
        int iRC = -1 ;
        if (m_group != null) {
            PCheckbox selected = m_group.getSelectedCheckbox() ;
            if (selected != null) {
                iRC = m_checkboxes.indexOf(selected) ;
            }
        }
        return iRC ;
    }


    /**
     * Return the group's selected option. An option is considered selected
     * if it is in the "on" or checked state.
     *
     * @return The selected option's label text or null if no items are selected.
     */
    public String getSelectedGroupOption()
    {
        String strRC = null ;

        if (m_group != null) {
            PCheckbox selected = m_group.getSelectedCheckbox() ;
            if (selected != null) {
                strRC = selected.getLabel() ;
            }
        }
        return strRC ;
    }


    /**
     * Add an option to this simple option form with the specified label,
     * state, and hint text.
     *
     * @param strOption The label text used to describe the option.
     * @param bState True for "on" or checked, or false for "off" or unchecked.
     * @param strHint The hint text to display when an end user holds
     *        down the button associated with this option. To include a title as well as text, use the
     *        format <hint title>|<hint text>.
     */
    public void addOption(String strOption, boolean bState, String strHint)
    {
        PCheckbox checkbox = new PCheckbox(strOption, bState, m_group, strHint) ;

        // Add it to our internal hash table and then to the container
        m_htCheckboxes.put(strOption, checkbox) ;
        m_checkboxes.addComponent(checkbox) ;
    }


    /**
     * Get the value of the specified option. If the specified option was
     * never added to this form, a silent error is detected and false is
     * returned.
     *
     * @param strOption The label/text used to describe the option.
     * @return boolean True for "on" or checked, or false for "off" or unchecked.
     */
    public boolean getOption(String strOption)
    {
        boolean bState = false ;

        PCheckbox checkbox = (PCheckbox) m_htCheckboxes.get(strOption) ;
        if (checkbox != null)
            bState = checkbox.getState() ;

        return bState ;
    }


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
                null, //only use global listener
                m_actionDispatcher.ACTION_OK) ;
        PActionItem menuActionOk = new PActionItem( new PLabel(getString("lblGenericOk")),
                getString("hint/xdk/simpleoptionform/ok"),
                m_actionDispatcher,
                m_actionDispatcher.ACTION_OK) ;
        menuControl.setItem(PBottomButtonBar.B3, actionOk) ;
        getLeftMenuComponent().addItem(menuActionOk) ;

        PActionItem actionCancel = new PActionItem( new PLabel(getString("lblGenericCancel")),
                getString("hint/xdk/simpleoptionform/cancel"),
                null, //only use global listener
                m_actionDispatcher.ACTION_CANCEL) ;
        PActionItem menuActionCancel = new PActionItem( new PLabel(getString("lblGenericCancel")),
                getString("hint/xdk/simpleoptionform/cancel"),
                m_actionDispatcher,
                m_actionDispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, actionCancel) ;
        getLeftMenuComponent().addItem(menuActionCancel) ;
    }


    /**
     * Initializes the checkbox container and adds it to the form.
     */
    private void initControls()
    {
        m_checkboxes = new PScrollableComponentContainer(4, PButtonEvent.BID_R1) ;
        addToDisplayPanel(m_checkboxes, new Insets(2,6,2,6)) ;
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
}
