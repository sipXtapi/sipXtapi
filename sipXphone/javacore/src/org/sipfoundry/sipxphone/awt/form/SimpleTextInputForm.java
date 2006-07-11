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
 * <p>A Simple Text Input Form prompts the end user for a single piece of
 * text input. Visually, the form contains (from top to bottom): an optional
 * label for the text field, a multi-mode text field, and an optional
 * instructions label. The bottom button bar is populated with a Cancel
 * button in the middle (B2) and an Ok button on the right (B3).</p>
 *
 * <p>The label defaults to blank; however, this form always reserves the top 25
 * percent of the form's area. Invoke <i>setLabel("")</i> to set the label.</p>
 *
 * <p>The text field also defaults to blank and should be set/queried by invoking
 * the <i>setText()</i> and <i>getText()</i> accessor methods. The text field
 * also supports
 * multiple text input modes. The text field can be placed into alphanumeric
 * mode, which allows users to enter letters, numbers, and punctuation
 * characters or into numeric mode where only numbers can be entered. The
 * default mode is alphanumeric and can be changed by invoking <i>setMode()</i>.</p>
 *
 * <p>The instruction label area defaults to blank and can be set by calling
 * the <i>setInstructions()</i> method. This area is multiline and will
 * automatically wrap your text. Additionally, a "\n" can be used to force a
 * line break.</p>
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SimpleTextInputForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Text field input mode that allows alphanumeric input: letters, numbers, and
        punctuation. */
    public static final int MODE_ALPHANUMERIC   = 0 ;
    /** Text field input mode that allows numeric input: numbers only. */
    public static final int MODE_NUMERIC        = 1 ;
    /** Text field input mode that allows for passwords. */
    public static final int MODE_PASSWORD       = 2 ;
    /** Text field input mode that allows for alpha-numeric passwords. */
    public static final int MODE_PASSWORD_TEXT  = 3 ;

    /** Constant value that is returned through <i>showModal()</i> when the user
        presses the 'Ok' button. */
    public static final int OK = 1 ;

    /** Constant value that is returned through <i>showModal()</i> when the user
        presses the 'Cancel' button. */
    public static final int CANCEL = 0 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private PCommandBar  m_cbActions ;  // Command bar displayed on right of screen.
    private PLabel       m_lblInput ;   // Label displayed above the input field.
    private PTextField   m_tfInput ;    // The actual text field.
    private PLabel       m_lblInstructions ;  // Instructions displayed below the input field.

    private PContainer   m_container ; // Container for all our components

    private icCommandDispatcher m_dispatcher ;  // command / action event dispatcher
    private TextFieldActionEventAdapter m_textfieldAdapter ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor, takes an application object and form title.
     *
     * @param application Reference to the invoking application.
     * @param strTitle The title of the form; displays in the title bar.
     */
    public SimpleTextInputForm(Application application,
                               String      strTitle)
    {
        super(application, strTitle) ;

        setTitle(strTitle) ;
        m_dispatcher = new icCommandDispatcher() ;

        // Create all of our components
        initControls() ;

        // physically lay everything out
        layoutComponents() ;
    }


    /**
     * Constructor, takes a parent form and form title.
     *
     * @param formParent Parent form responsible for displaying this sub-form.
     * @param strTitle The title of the form; displays in the title bar.
     */
    public SimpleTextInputForm(PForm formParent,
                               String strTitle)
    {
        super(formParent, strTitle) ;

        setTitle(strTitle) ;
        m_dispatcher = new icCommandDispatcher() ;

        // Create all of our components
        initControls() ;

        // physically lay everything out
        layoutComponents() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Invoked when the user presses the middle (B2) Cancel button.
     * Simply closes the form by invoking <i>closeForm</i>.
     */
    public void onCancel()
    {
        closeForm(CANCEL) ;
    }


    /**
     * Invoked when the user presses the right (B3) Ok button. Simply
     * closes the form by invoking <i>closeForm</i>.
     */
    public void onOk()
    {
        closeForm(OK) ;
    }


    /**
     * Set the data entry mode of this text input form. The mode can be set
     * to either MODE_ALPHANUMERIC or MODE_NUMERIC. Any other value will
     * result in an IllegalArgumentException being thrown.
     *
     * @param iMode The desired mode of the text input field. Specify
     *        MODE_ALPHANUMERIC for mixed numeric, alpha-numeric, and
     *        punctuation characters or MODE_NUMERIC for numeric
     *        characters only.
     */
    public void setMode(int iMode)
    {
        switch (iMode) {
            case MODE_ALPHANUMERIC:
                m_tfInput.setAlphanumericMode(true) ;
                m_tfInput.clearEchoChar() ;
                m_textfieldAdapter.enableCapslock(true, m_tfInput) ;
                m_textfieldAdapter.setAllowNumericMode(true);
                break ;
            case MODE_NUMERIC:
                m_tfInput.setAlphanumericMode(false) ;
                m_tfInput.clearEchoChar() ;
                m_textfieldAdapter.enableCapslock(false, m_tfInput) ;
                break ;
            case MODE_PASSWORD:
                m_tfInput.setAlphanumericMode(false) ;
                m_textfieldAdapter.enableCapslock(false, m_tfInput) ;
                m_textfieldAdapter.setAllowNumericMode(true);
                m_tfInput.setEchoChar('*') ;
                m_textfieldAdapter.enableButtonByAction(m_textfieldAdapter.ACTION_BACKWARD,false);
                break ;
            case MODE_PASSWORD_TEXT:
                m_tfInput.setAlphanumericMode(true) ;
                m_textfieldAdapter.enableCapslock(true, m_tfInput) ;
                m_textfieldAdapter.setAllowNumericMode(true);
                m_tfInput.setEchoCharDelay('*') ;
                m_textfieldAdapter.enableButtonByAction(m_textfieldAdapter.ACTION_BACKWARD,false);
                break ;
            default:
                throw new IllegalArgumentException("invalid mode identifier (" + Integer.toString(iMode) + ")") ;
        }
    }


    /**
     * Get the form's data input mode.
     *
     * @return MODE_ALPHANUMERIC if the the form is configured to accept mixed
     *         numeric, alphanumeric, and punctuation characters or
     *         MODE_NUMERIC if the form is configured to only accept numbers.
     */
    public int getMode()
    {
        int iMode = MODE_NUMERIC ;

        if (m_tfInput.getAlphanumericMode())
            iMode = MODE_ALPHANUMERIC ;

        return iMode ;
    }


    /**
     * Set the initial text for the text edit control.
     *
     * @param strText The text string that will be displayed (and edited)
     *        in the text edit control.
     */
    public void setText(String strText)
    {
        m_tfInput.setText(strText) ;
    }


    /**
     * Get the text in the text edit control.
     *
     * @return String Text as displayed in the text edit control.
     */
    public String getText()
    {
        return m_tfInput.getText() ;
    }


    /**
     * Set the text field label displayed above the text edit control. The
     * text field currently is limited to a single line of text and is left
     * justified.
     *
     * @param strLabel The label string that will be displayed above the text
     *        edit control.
     */
    public void setLabel(String strLabel)
    {
        m_lblInput.setText(strLabel) ;
    }


    /**
     * Set the instructional label to display below the text edit control.
     * The instructions area can display multiple lines of text and will
     * automatically wrap on white space. You can insert a line break by
     * inserting a "\n" into the string.
     *
     * @param strInstructions The instructional string to display
     *        below the text edit control.
     */
    public void setInstructions(String strInstructions)
    {
        m_lblInstructions.setText(strInstructions) ;
    }


    /**
     * Add a text listener to this text input form. Text listeners receive
     * notification whenever the text within the text field component is
     * changed. If the specified listener has already been added, the add is
     * ignored with a silent error.
     *
     * @see org.sipfoundry.sipxphone.awt.event.PTextListener
     *
     * @param listener The PTextListener that will receive update/change
     *        notifications.
     */
    public void addTextListener(PTextListener listener)
    {
        m_tfInput.addTextListener(listener) ;
    }


    /**
     * Remove a text listener from this text input form. If the specified
     * text listener is not listening, the request is ignored with a silent
     * error.
     *
     * @param listener The PTextListener that should stop receiving update/change
     *        notifications.
     */
    public void removeTextListener(PTextListener listener)
    {
        m_tfInput.removeTextListener(listener) ;
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
     * can be inserted by placing a "\n" into text string.
     *
     * @param strHelp The help string to display on this form's
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
     * form can display two columns of menu tabs. See the xDK
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
     * form can display two columns of menu tabs. See the xDK
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

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    private void initControls()
    {
        //add global listener
        getBottomButtonBar().addActionListener(m_dispatcher);

        m_lblInput = new PLabel("") ;
        m_lblInput.setAlignment(PLabel.ALIGN_WEST | PLabel.ALIGN_SOUTH) ;
        m_tfInput = new PTextField("") ;
        m_lblInstructions = new PLabel("", PLabel.ALIGN_NORTH) ;

        m_cbActions = new PCommandBar() ;
        m_container = new PContainer() ;

        initMenus() ;

        m_textfieldAdapter = new TextFieldActionEventAdapter(m_tfInput, m_cbActions, false) ;
        m_textfieldAdapter.populateCommandbar() ;
    }


    /**
     *
     */
    private void initMenus()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        PActionItem actionOk = new PActionItem( new PLabel(getString("lblGenericOk")),
                getString("hint/xdk/simpletextinputform/ok"),
                null,
                m_dispatcher.ACTION_OK) ;
        PActionItem menuActionOk = new PActionItem( new PLabel(getString("lblGenericOk")),
                getString("hint/xdk/simpletextinputform/ok"),
                m_dispatcher,
                m_dispatcher.ACTION_OK) ;

        menuControl.setItem(PBottomButtonBar.B3, actionOk) ;
        getLeftMenuComponent().addItem(menuActionOk) ;

        PActionItem actionCancel = new PActionItem( new PLabel(getString("lblGenericCancel")),
                getString("hint/xdk/simpletextinputform/cancel"),
                null,
                m_dispatcher.ACTION_CANCEL) ;
        PActionItem menuActionCancel = new PActionItem( new PLabel(getString("lblGenericCancel")),
                getString("hint/xdk/simpletextinputform/cancel"),
                m_dispatcher,
                m_dispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, actionCancel) ;
        getLeftMenuComponent().addItem(menuActionCancel) ;
    }


    /**
     * @deprecated do not expose
     */
    public void doLayout()
    {
        Dimension dimSize = m_container.getSize() ;

        m_container.doLayout() ;

        if (m_cbActions != null) {
            m_cbActions.setBounds(0, 0, 27, dimSize.height) ;
            m_cbActions.doLayout() ;
        }

        if (m_lblInput != null) {
            m_lblInput.setBounds(27, 0, dimSize.width-30, 27) ;
            m_lblInput.doLayout() ;
        }

        if (m_tfInput != null) {
            m_tfInput.setBounds(27, 27, dimSize.width-30, 27) ;
            m_tfInput.doLayout() ;
        }

        if (m_lblInstructions != null) {
            m_lblInstructions.setBounds(30, 27+27, dimSize.width-33, dimSize.height-(27+27+4)) ;
            m_lblInstructions.doLayout() ;
        }

        super.doLayout() ;
    }


    /**
     *
     */
    private void layoutComponents()
    {
        setLayout(null) ;
        m_container.setLayout(null) ;

        m_container.add(m_cbActions) ;
        m_container.add(m_lblInput) ;
        m_container.add(m_tfInput) ;
        m_container.add(m_lblInstructions) ;

        addToDisplayPanel(m_container, new Insets(1, 1, 1, 1)) ;
    }


    /**
     *
     */
    private class icCommandDispatcher implements PActionListener
    {
        protected String ACTION_OK        = "action_ok" ;
        protected String ACTION_CANCEL    = "action_cancel" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_OK))
                onOk() ;
            else if (event.getActionCommand().equals(ACTION_CANCEL))
                onCancel() ;
        }
    }
}

