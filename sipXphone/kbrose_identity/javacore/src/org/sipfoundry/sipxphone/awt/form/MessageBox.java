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

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;


/**
 * A message box allows an application to display a modal warning, error, or
 * informational message. You can set the text of the message as well as an
 * optional title. The message title defaults to "Warning", "Error", or "Information"
 * depending on its type.
 *
 * You can also change the default button labels. These labels are:
 * <pre>
 *  Warning:
 *     [         ] [ CANCEL  ] [    OK   ]
 *  Error:
 *     [         ] [         ] [  CLOSE  ]
 *  Information:
 *     [         ] [         ] [    OK   ]
 * </pre>
 *
 * The return corresponds to the button that is pressed: LEFT, MIDDLE, or
 * RIGHT. CANCEL, OK, and CLOSE have
 * also been set to match the other constants.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class MessageBox extends PAbstractForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** This message box contains information only. Defaults to a single action: OK. */
    public static final int TYPE_INFORMATIONAL = 0 ;
    /** This message box contains a warning and solicits an action. The user must choose between
      * two (or possibly more) actions. Default actions are OK and Cancel. */
    public static final int TYPE_WARNING       = 1 ;
    /** This message box indicates that something very bad happened. Defaults
      * to a single action: Close. */
    public static final int TYPE_ERROR         = 2 ;

    /** The left-most bottom button was pressed. */
    public static final int B1           = 0 ;
    /** The middle bottom button was pressed. */
    public static final int B2           = 1 ;
    /** The right-most bottom button was pressed. */
    public static final int B3           = 2 ;
    /** The user pressed the OK (right) button. */
    public static final int OK           = 2 ;
    /** The user pressed the Cancel (middle) button. */
    public static final int CANCEL       = 1 ;
    /** The user pressed the Close (right) button. */
    public static final int CLOSE        = 2 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** title bar control */
    private PTitlebar   m_titlebar ;

    /** display area for the message box text */
    private PDisplayArea m_displayArea ;

    /** menu control */
    private PBottomButtonBar m_menuControl ;

    /** where we actually place our data*/
    private PBorderedContainer m_displayPane ;

    /** our actual button labels */
    protected PLabel m_lblButtons[] = new PLabel[3] ;
    protected String m_strHints[] = new String[3] ;


    /** what type of control is this?  (INFORMATION, ERROR, WARNING) */
    protected int    m_iType ;

    /** the title we will display upon <i>showModal</i> */
    protected String m_strTitle = null ;

    /** the message that we will display upon <i>showModal</i> */
    protected String m_strMessage = null ;


//////////////////////////////////////////////////////////////////////////////
// Constructions
////

    /**
     * Constructor requiring an application reference. The assumed type is
     * TYPE_INFORMATIONAL.
     *
     * @see #TYPE_INFORMATIONAL
     */
    public MessageBox(Application application)
    {
        this(application, TYPE_INFORMATIONAL) ;
    }


    /**
     * Constructor requiring a parent form.  The assumed type is
     * TYPE_INFORMATIONAL.
     *
     * @see #TYPE_INFORMATIONAL
     */
    public MessageBox(PForm formParent)
    {
        this(formParent, TYPE_INFORMATIONAL) ;
    }


    /**
     * Constructor requiring an application reference and message box type.
     *
     * @see #TYPE_INFORMATIONAL
     * @see #TYPE_WARNING
     * @see #TYPE_ERROR
     */
    public MessageBox(Application application, int iType) throws IllegalArgumentException
    {
        super(application) ;
        if (application == null)
            throw new IllegalArgumentException("Application passed as null.");

        setFormName("MessageBox") ;

        createComponents() ;
        layoutComponents() ;

        setType(iType) ;
    }


    /**
     * Constructor requiring a parent form and message box type.
     *
     * @see #TYPE_INFORMATIONAL
     * @see #TYPE_WARNING
     * @see #TYPE_ERROR
     */
    public MessageBox(PForm formParent, int iType) throws IllegalArgumentException
    {
        super(formParent.getApplication(), formParent) ;

        setFormName("MessageBox") ;

        createComponents() ;
        layoutComponents() ;

        setType(iType) ;
    }



     /**
      * Set the type of message box to display
      * NOTE: This <b>will</b> reset the title and buttons to their default form.
      *
      * @since 0.7.0
      *
      * @see none
      * @see none
      *
      * @param int iType
      * TYPE_INFORMATIONAL, TYPE_WARNING, and TYPE_ERROR.</p>
      *
      * @return void
      * @exception IllegalArgumentException
      * @author Daniel Winsor
      */
    public void setType(int iType) throws IllegalArgumentException
    {
        m_iType = iType ;

        switch (iType) {
            case TYPE_INFORMATIONAL:
                m_strTitle = getString("lblAlertDefaultInformationalTitle") ;
                m_lblButtons[B1] = null ;
                m_strHints[B1] = null ;
                m_lblButtons[B2] = null ;
                m_strHints[B2] = null ;
                m_lblButtons[B3] = new PLabel(getString("lblAlertOkButton")) ;
                m_strHints[B3] = getString("hint/core/messagebox/information_ok") ;
                m_titlebar.setIcon(getImage("imgInfoIcon")) ;
                break ;
            case TYPE_WARNING:
                m_strTitle = getString("lblAlertDefaultWarningTitle") ;
                m_lblButtons[B1] = null ;
                m_strHints[B1] = null ;
                m_lblButtons[B2] = new PLabel(getString("lblAlertCancelButton")) ;
                m_strHints[B2] = getString("hint/core/messagebox/warning_cancel") ;
                m_lblButtons[B3] = new PLabel(getString("lblAlertOkButton")) ;
                m_strHints[B3] = getString("hint/core/messagebox/warning_ok") ;
                m_titlebar.setIcon(getImage("imgWarningIcon")) ;
                break ;
            case TYPE_ERROR:
                m_strTitle = getString("lblAlertDefaultErrorTitle") ;
                m_lblButtons[B1] = null ;
                m_strHints[B1] = null ;
                m_lblButtons[B2] = null ;
                m_strHints[B2] = null ;
                m_lblButtons[B3] = new PLabel(getString("lblAlertCloseButton")) ;
                m_strHints[B3] = getString("hint/core/messagebox/error_close") ;
                m_titlebar.setIcon(getImage("imgErrorIcon")) ;
                break ;
            default:
                throw new IllegalArgumentException("Invalid type passed to MessageBox");

        }
    }


    /**
     * Set the button label for the specified position.
     *
     * @param iPosition The button position where the specified label will
     *        be placed.
     * @param strLabel The string label to display at the
     *        specified position.
     * @param strHint The hint text to display when a user presses down
     *        and holds this button. To include a title as well as text, use the
     *        format <hint title>|<hint text>.
     *
     * @exception IllegalStateException Thrown if iPosition is invalid.
     */
    public void setButton(int iPosition, String strLabel, String strHint)
    {

         // Make sure the position is valid
        if ((iPosition < B1) || (iPosition > B3)) {
            throw new IllegalStateException("invalid position index") ;
        }

        // Perform clean up if we are replacing a button
        if (m_lblButtons[iPosition] != null) {
            m_menuControl.clearItem(iPosition) ;
        }

        m_lblButtons[iPosition] = new PLabel(strLabel) ;
        m_strHints[iPosition] = strHint ;

        switch (iPosition) {
            case B1:
                m_menuControl.setItem(iPosition, m_lblButtons[iPosition], "action_button_left", m_strHints[iPosition]) ;
                break ;
            case B2:
                m_menuControl.setItem(iPosition, m_lblButtons[iPosition], "action_button_middle", m_strHints[iPosition]) ;
                break ;
            case B3:
                m_menuControl.setItem(iPosition, m_lblButtons[iPosition], "action_button_right", m_strHints[iPosition]) ;
                break ;
        }
    }



    /**
     * Display this form in a modeless state where control is immediately
     * returned to the caller (non-blocking). Places a form that is already
     * displayed but not active into focus.
     *
     * @return True if a new form is displayed; else false.
     */
    public synchronized boolean showModeless()
    {
        initializeMessageBox() ;

        return super.showModeless() ;
    }


    /**
     * Display this form in a modal state where control is not returned
     * to the caller until the form is closed by the end user. If the form
     * is already being displayed, then an IllegalStateException exception is
     * thrown.
     *
     * @return The value specified when invoking closeForm.
     *
     * @exception IllegalStateException Thrown if the form is already being
     *            displayed.
     */
    public synchronized int showModal()
    {
        initializeMessageBox() ;

        return super.showModal() ;
    }


    /**
     * Display this form in a modal state where control is not returned
     * to the caller until the form is closed by the end user. If the form
     * is already being displayed, then an IllegalStateException exception is
     * thrown.
     *
     * @return The value specified when invoking closeForm.
     *
     * @exception IllegalStateException Thrown if the form is already being
     *            displayed.
     *
     * @deprecated do not expose yet
     */
    public synchronized int showModal(boolean bAlwaysOnTop)
    {
        initializeMessageBox() ;

        return super.showModal(bAlwaysOnTop) ;
    }



    /**
     * Set the message box title. The default title is derived from the
     * message box type: "Warning", "Error", or "Information".
     *
     * @param strTitle New title for the message box.
     */
    public void setTitle(String strTitle)
    {
        m_strTitle = strTitle ;
    }


    /**
     * Set the message box text. The message box will automatically wrap your
     * text on white space, or you can insert "\n" into the text to explicitly
     * force a line break. If needed, this control automatically adds a scroll
     * bar to support scrolling.
     *
     * @param strMessage Message box text.
     */
    public void setMessage(String strMessage)
    {
        //m_displayArea.setString(strText) ;
        m_strMessage = strMessage ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected void initializeMessageBox()
    {
        m_titlebar.setTitle(m_strTitle) ;
        m_displayArea.setText(m_strMessage) ;

        // Left Button
        if (m_lblButtons[B1] != null) {
            m_menuControl.setItem(PBottomButtonBar.B1, m_lblButtons[B1], "action_button_left", m_strHints[B1]) ;
        }

        // Middle Button
        if (m_lblButtons[B2] != null) {
            m_menuControl.setItem(PBottomButtonBar.B2, m_lblButtons[B2], "action_button_middle", m_strHints[B2]) ;
        }

        // Right Button
        if (m_lblButtons[B3] != null) {
            m_menuControl.setItem(PBottomButtonBar.B3, m_lblButtons[B3], "action_button_right", m_strHints[B3]) ;
        }
    }


    /**
     *
     */
    protected void createComponents()
    {
        AppResourceManager resMgr = AppResourceManager.getInstance() ;

        // Create Titlebar
        m_titlebar = new PTitlebar(PTitlebar.MESSAGEBOX_FRAME) ;
        m_titlebar.setIcon(resMgr.getImage("imgWarningIcon")) ;

        m_menuControl = new PBottomButtonBar() ;
        m_menuControl.addActionListener(new icActionDispatcher()) ;

        m_displayPane = new PBorderedContainer(PBorderedContainer.MESSAGEBOX_BORDER) ;
        m_displayArea = new PDisplayArea() ;
        m_displayArea.setTextAlignment(PDisplayArea.ALIGN_CENTER) ;
    }


    /**
     *
     */
    protected void layoutComponents()
    {
        setLayout(null) ;

        m_titlebar.setBounds(0, 0, 160, 26) ;
        add(m_titlebar) ;

        m_displayPane.setBounds(0, 26, 160, 108) ;
        m_displayPane.setInsets(new Insets(0, 8, 0, 8)) ;
        add(m_displayPane) ;

        m_displayPane.add(m_displayArea) ;

        m_menuControl.setBounds(0, 134, 160, 26) ;
        add(m_menuControl) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner classes
////

    /**
     * listen for button presses
     */
    protected class icActionDispatcher implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals("action_button_left")) {
                closeForm(B1) ;
            } else if (event.getActionCommand().equals("action_button_middle")) {
                closeForm(B2) ;
            } else if (event.getActionCommand().equals("action_button_right")) {
                closeForm(B3) ;
            }
        }
    }
}
