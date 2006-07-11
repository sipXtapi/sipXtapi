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


/**
 * SimpleTextForm is a simple container designed to display text and
 * present the users with three possible actions. These actions correspond
 * to the three screen display buttons at the bottom of the form.
 * <p>
 * Text to label each action should be set using the <i>setText(String)</i>
 * method. The embedded PDisplayArea control will wrap text automatically;
 * however, you can also place "\n" in your string to explicitly force
 * a line break.
 * <p>
 * The Bottom Button Bar presents actions or choices to the user.
 * Use the <i>getBottomButtonBar</i> method to gain a reference to
 * the button bar. Afterwards, use the <i>setItem</i> method to set
 * actions.
 * <p>
 * By default, the Bottom Button Bar is initialized with a "Cancel" action
 * assigned to the center (B2) button and an "Ok" action assigned to the right
 * (B3) button. Pressing either of these button closes the form by
 * invoking <i>closeForm()</i>.
 *
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SimpleTextForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Exit code return from showModal() when the form is closed by pressing
        the OK Button */
    public static final int OK = 1 ;
    /** Exit code return from showModal() when the form is closed by pressing
        the Cancel Button */
    public static final int CANCEL = 0 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** The read-only text area (a.k.a display area) control. */
    private PDisplayArea m_daStats ;

    private icActionDispatcher m_actionDispatcher;
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor, takes an application object and form title.
     *
     * @param application Reference to the invoking application.
     * @param strTitle The title of the form to display in the title bar.
     */
    public SimpleTextForm(Application application,
                          String      strTitle)
    {
        super(application, strTitle) ;

        setTitle(strTitle) ;

        // Create all of our components
        initControls() ;
        initMenus() ;

        // physically lay everything out
        layoutComponents() ;
    }


    /**
     * Constructor, takes a parent form and form title.
     *
     * @param formParent Parent form responsible for displaying this sub-form.
     * @param strTitle The title of the form to display in the title bar.
     */
    public SimpleTextForm(PForm  formParent,
                          String  strTitle)
    {
        super(formParent, strTitle) ;

        setTitle(strTitle) ;

        // Create all of our components
        initControls() ;
        initMenus() ;

        // physically lay everything out
        layoutComponents() ;
    }



    /**
     * <i>onCancel</i> is called when the user presses the middle (B2) Cancel
     * button. Simply closes the form by invoking <i>closeForm</i>.
     */
    public void onCancel()
    {
        closeForm(CANCEL) ;
    }


    /**
     * <i>onOk</i> is called when the user presses the right (B3) Ok
     * button. Simply closes the form by invoking <i>closeForm</i>.
     */
    public void onOk()
    {
        closeForm(OK) ;
    }


    /**
     * Populate the PDisplayArea with the supplied text. The text
     * will automatically be wrapped for you, however, you can also
     * insert "\n" to force a line break.
     *
     * @param strText The text you wish to display, or null to clear.
     */
    public void setText(String strText)
    {
        if (strText != null)
            m_daStats.setText(strText) ;
        else
            m_daStats.setText(null) ;
    }

    /**
     * Close this form with the specified exit code. This exit code is
     * returned to the form invoker as the return value of <i>showModal</i>.
     *
     * @param iExitCode The exit code of the form.
     *
     * @see #showModal
     *
     */
    public void closeForm(int iExitCode)
    {
        super.closeForm(iExitCode) ;
    }


    /**
     * Set the help text that will be display on this form's help tab.
     * The help text wraps automatically; however, explicit new lines
     * can be inserted by placing "\n" into the text string.
     *
     * @param strHelp The help string that will display on this form's
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
     * <u>Guidelines for User Interface Design</u> for recommendations on
     * menu placement and balancing.
     *
     * @param items An array of PActionItems where each PActionItem represents
     *        a single menu item.
     *
     * @see org.sipfoundry.sipxphone.awt.PActionItem
     *
     */
    public void setLeftMenu(PActionItem[] items)
    {
        super.setLeftMenu(items) ;
    }


    /**
     * Set the right column of menu items for this form's menu tab. Each
     * form can display two columns of menu tabs. Please see the xDK
     * <u>Guidelines for User Interface Design</u> for recommendations on
     * menu placement and balancing.
     *
     * @param items An array of PActionItems where each PActionItem represents
     *        a single menu item.
     *
     * @see org.sipfoundry.sipxphone.awt.PActionItem
     *
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
     * Sets strWrappingDelimiters as the wrapping delimiters for the
     * display area of this SimpleTextForm. The default delimiters used are  " \t-\\/:,."
     * i.e. space, tab, hyphen, backslash, forwardslash, colon, comma and
     * a period. Call this  method if you like to set the wrapping delimiters
     * to something other than the default ones.
     *
     * @param strWrappingDelimiters String consisting of delimiters used as
     * breaking points while wrapping.
     *
     * @deprecated Do not expose yet.
     */
    public void setWrappingDelimiters(String strWrappingDelimiters ){
        m_daStats.setWrappingDelimiters(strWrappingDelimiters);
    }
//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     *
     */
    private void initControls()
    {
        m_daStats = new PDisplayArea() ;

        //add global listener
        m_actionDispatcher = new icActionDispatcher() ;
        getBottomButtonBar().addActionListener(m_actionDispatcher);
    }


    /**
     *
     */
    private void initMenus()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        PActionItem actionOk = new PActionItem( new PLabel(getString("lblGenericOk")),
                getString("hint/xdk/simpletextform/ok"),
                null, //use global for this button
                "ACTION_OK") ;
        PActionItem menuActionOk = new PActionItem( new PLabel(getString("lblGenericOk")),
                getString("hint/xdk/simpletextform/ok"),
                m_actionDispatcher,
                "ACTION_OK") ;
        menuControl.setItem(PBottomButtonBar.B3, actionOk) ;
        getLeftMenuComponent().addItem(menuActionOk) ;

        PActionItem actionCancel = new PActionItem( new PLabel(getString("lblGenericCancel")),
                getString("hint/xdk/simpletextform/cancel"),
                null, //use global for this button
                "ACTION_CANCEL") ;
        PActionItem menuActionCancel = new PActionItem( new PLabel(getString("lblGenericCancel")),
                getString("hint/xdk/simpletextform/cancel"),
                m_actionDispatcher,
                "ACTION_CANCEL") ;
        menuControl.setItem(PBottomButtonBar.B2, actionCancel) ;
        getLeftMenuComponent().addItem(menuActionCancel) ;
    }


    /**
     */
    private void layoutComponents()
    {
        addToDisplayPanel(m_daStats, new Insets(1, 1, 1, 1)) ;
    }


    /**
     *
     */
    private class icActionDispatcher implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals("ACTION_CANCEL"))
                onCancel() ;
            if (event.getActionCommand().equals("ACTION_OK"))
                onOk() ;
        }
    }

}

