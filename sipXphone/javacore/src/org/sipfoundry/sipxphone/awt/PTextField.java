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


package org.sipfoundry.sipxphone.awt ;

import java.awt.* ;
import java.awt.event.* ;
import java.util.Vector ;
import java.util.Enumeration ;
import java.awt.datatransfer.* ;
import java.lang.Character;

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.util.* ;


/**
 * A text field component is a text componentat that allows the end user to
 * edit a single line of text in either numeric or alphanumeric mode.
 * <p>
 * "numeric mode" may be enabled by invoking the 'setAlphanumericMode()'
 * method with a boolean parameter of 'false'.  After enabling, the text
 * field will only collect numberic digits and the star '*' and pound '#'
 * characters.
 * <p>
 * "alphanumeric mode" may be enabled by invoking the 'setAlphanumericMode()'
 * method with a boolean parameter of 'true'.  After enabling, the text
 * field will collect both numeric digits and alphanumeric characters by a
 * cycling mechanism.  For example, pressing the '2' key multiple times, will
 * cycle through the the following characters: 'a', 'b', 'c', '2'.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PTextField extends PComponent implements PFocusTraversable
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** table holding rotation sequence for alphanumeric mode */
    private static final char DEF_ALPHANUMERIC_KEYS[][] =
        {
            {'=', '0', '@', '.', '/', ':', '-', '?', '!', ',', '\"', '\'', '&', '$', '(', ')', '%', '_', '<', '>', ' ', ';'},
            {'1'},
            {'2','a','b','c'},
            {'3','d','e','f'},
            {'4','g','h','i'},
            {'5','j','k','l'},
            {'6','m','n','o'},
            {'7','p','q','r','s'},
            {'8','t','u','v'},
            {'9','w','x','y','z'},
            {'.','*'},
            {'#'},
        } ;

    protected static final int CTRL_C = 0x03;

    protected static final int CTRL_V = 0x16;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** font used to when displaying text (without a label) */
    private Font m_displayFont       = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT) ;
    /** font used to when displaying text (with a label) */
    private Font m_displayLabelFont  = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL) ;

    /** string current within the editor */
    private String m_strText           = "" ;
    /** the current caret position */
    private int m_iCaretPosition    = 0 ;
    /** previous caret position - allows us to determine if caret moved */
    private int m_iLastCaretPosition = -1 ;
    /** has timer advanced the caret? */
    private boolean m_bTimerAdvanced = false ;

    /** are we in alpha numeric mode or simple numeric mode? */
    private boolean m_bAlphaNumericMode = true ;

    /** last keycode entered in alphanumeric mode */
    private int m_iLastKey          = -1 ;
    /** last alpha index number (used for rotation) */
    private int m_iLastAlphaIndex   = 0 ;

    /* Should the user enter all of thier data in capital letters? */
    private boolean m_bCapsLock = false ;

    /*Should the component enter only IP characters*/
    private boolean m_bNumericMode = false;

    /* are we currently cycling through alphanumber entries? */
    private boolean m_bCycling = false ;

    /** timeout handler so that we advance the caret after xxx ms */
    private icAlphanumericTimeout m_timeoutHandler = new icAlphanumericTimeout() ;

    /** echo character to display if requested */
    private char m_cEchoChar ;
    /** have we been requested to display an echo character instead of straight text? */
    private boolean m_bEchoCharIsSet ;
    /** Do we display current char in clear text, previous chars as echo character */
    private boolean  m_bEchoDelayIsSet ;

    /** Default length of text field in characters */
    private int m_iTextFieldLength ;
    /** Should we automatically calculate the text field length? */
    private boolean m_bAutoTextFieldLength ;

    private int m_iTextFieldHeight ;
    private int m_iTextFieldWidth  ;
    private int m_iTextFieldXOffset ;
    private int m_iTextFieldYOffset  ;

    private int m_iTextFieldHScroll ;

    private Object m_objMutex = new Object();

    private boolean m_bClearLastKey ;

    private Vector m_vFocusListeners ;
    private Vector m_vTextListeners ;

    /** does this control have focus? */
    private boolean m_bHasFocus ;
    /** is this textfield read-only? */
    private boolean m_bReadOnly ;
    /** is this textfield enabled or disabled? */
    private boolean m_bEnabled ;

    /** each text field can have user label */
    private String m_strLabel ;
    /** the user label must be aligned relatively to the text field */
    private int m_iLabelAlignment ;

    /** X shift value for the text field. */
    private int m_iAlignXOffset = 0 ;
    /** Y shift value for the text field. */
    private int m_iAlignYOffset = 0 ;

    /** Should echo character delay? - this is for displaying password *s */
    private boolean m_bDelay = false ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Construct a blank text field.
     */
    public PTextField()
    {
        this("") ;
    }


    /**
     * Construct a blank text field of a specifed length.
     *
     * @param iLength Desired length of text field in characters or -1 to use
     *        default text field length.
     */
    public PTextField(int iLength)
    {
        this("", iLength) ;
    }


    /**
     * Construct a text field with a specified text string.
     *
     * @param strText The text string to display initially in this
     *        text field.
     */
    public PTextField(String strText)
    {
        this(strText, -1) ;
    }


    /**
     * Construct a text field with a specified text string and a specified
     * length.
     *
     * @param strText The text string to display initially in this
     *        text field.
     * @param iLength Desired length of this text field in characters or -1 to use
     *        the default text field length.
     */
    public PTextField(String strText, int iLength)
    {
        m_strText = strText ;
        m_iCaretPosition = strText.length() ;

        addButtonListener(new icButtonListener()) ;
        setBackgroundImage(getImage("imgEditFrame")) ;

        m_iTextFieldHeight = -1;
        m_iTextFieldWidth = -1 ;
        m_iTextFieldXOffset = -1 ;
        m_iTextFieldYOffset = -1 ;
        m_iTextFieldHScroll = 0 ;
        m_bHasFocus = true ;
        m_bEnabled = true ;
        m_bReadOnly = false ;
        m_bEchoCharIsSet = false ;
        m_bEchoDelayIsSet = false ;
        m_strLabel = null ;
        m_iLabelAlignment = 0 ;

        // Initialize Text Field Length
        if (iLength <= 0) {
            m_bAutoTextFieldLength = true ;
            m_iTextFieldLength = -1 ;
        } else {
            m_bAutoTextFieldLength = false ;
            m_iTextFieldLength = iLength ;
        }
        enableDoubleBuffering(true) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Set the text to display in this text field.
     *
     * @param strText The text to display in this text field.
     */
    public synchronized void setText(String strText)
    {
        String strOldText = m_strText ;

        if (strText == null)
            strText = "" ;
        m_bCycling = false ;

        synchronized (m_objMutex) {
            m_iTextFieldHScroll = 0 ;
            m_strText = strText ;
            m_iCaretPosition = strText.length() ;
        }
        repaint() ;

        // Inform text listeners if the string has actually changed
        if (strText != null) {
            if (!strText.equals(strOldText))
                fireTextUpdate() ;
        } else if (strOldText != null)
            fireTextUpdate() ;
    }


    /**
     * Get the text in this text field.
     *
     * @return This text field's text content.
     */
    public synchronized String getText()
    {
        return m_strText ;
    }

    /**
     * Set the position of the caret (cursor) within the editor.
     *
     * @param iPosition The new position of the caret.
     */
    public synchronized void setCaretPosition(int iPosition)
    {
        synchronized (m_objMutex) {
            int iOldPosition = m_iCaretPosition ;

            m_iCaretPosition = iPosition ;
            m_bCycling = false ;

            // Clip along lower bound
            if (m_iCaretPosition < 0)
                m_iCaretPosition = 0 ;

            // Clip along upper bound
            if (m_iCaretPosition > m_strText.length())
                m_iCaretPosition = m_strText.length() ;

            // Only redraw if needed
            if (m_iCaretPosition != iOldPosition) {
                repaint() ;
                Timer.getInstance().removeTimers(m_timeoutHandler) ;

                fireCursorUpdate() ;
            }
        }
    }


    /**
     * Get the position of the caret (cursor).
     *
     * @return The current index position of the caret.
     */
    public int getCaretPosition()
    {
        return m_iCaretPosition ;
    }


    /**
     * Delete the character at the current caret position.
     */
    public synchronized void delete()
    {
        // Make sure we are within bounds
        synchronized (m_objMutex) {
            if ((m_iCaretPosition >= 0) && (m_iCaretPosition < m_strText.length())) {
                m_strText = m_strText.substring(0, m_iCaretPosition) + m_strText.substring(m_iCaretPosition+1) ;
                m_iLastKey = -1 ;
                m_bCycling = false ;

                fireTextUpdate() ;
            }
        }
    }

    /**
     * Move cursor to beginning of line
     */
    public synchronized void Home()
    {
        m_bCycling = false;
        m_iTextFieldHScroll = 0;
        setCaretPosition(0);
    }

    /**
     * Move cursor to end of line
     */
    public synchronized void End()
    {
        m_bCycling = false;
        int itextLength = m_strText.length();
        setCaretPosition(itextLength);
    }

    /**
     * Backspace one character at the current caret position.
     */
    public synchronized void backspace()
    {
        synchronized (m_objMutex) {

            boolean bWasCycling = m_bCycling;


            if (m_bCycling) {
                advanceCaret() ;
            }

            // Make sure we are within bounds
            if ((m_iCaretPosition > 0) && (m_iCaretPosition <= m_strText.length())) {
                m_bCycling = false ;
                m_strText = m_strText.substring(0, m_iCaretPosition-1) + m_strText.substring(m_iCaretPosition) ;
                moveCaretBackward() ;
                fireTextUpdate() ;
            }
        }
    }


    /**
     * Move the caret (cursor) position forward by one.
     */
    public synchronized void moveCaretForward()
    {
        m_bCycling = false ;
        m_iLastKey = -1 ;

        if (m_bAlphaNumericMode) {
            if (getCaretPosition() == m_strText.length()) {
                insertCharacter(' ') ;
            }
        }

        advanceCaret() ;
    }


    /**
     * Move the caret (cursor) position forward by one.
     */
    public synchronized void advanceCaret()
    {
        m_bCycling = false ;
        m_iLastKey = -1 ;

        setCaretPosition(getCaretPosition()+1) ;
    }



    /**
     * Move the caret position backward by one.
     */
    public synchronized void moveCaretBackward()
    {
        m_bCycling = false ;
        m_iLastKey = -1 ;
        setCaretPosition(getCaretPosition()-1) ;
    }


    /**
     * Set the echo character for this text field. An echo character is
     * useful for text fields where user input should not be shown on the
     * screen, as in the case of a text field for entering a password.
     *
     * @param c The echo character for this text field.
     */
    public synchronized void setEchoChar(char c)
    {
        m_cEchoChar = c ;
        m_bEchoCharIsSet = true ;
        repaint() ;
    }

    /**
     * Set an echo delay for the text field. This is the case where the
     * password is displayed in clear text when entered by the user,
     * however previously entered fields will be masked by the echo char.
     *
     * @param c The echo character for this text field
     */
    public synchronized void setEchoCharDelay(char c)
    {
        setEchoChar(c) ;
        m_bEchoDelayIsSet = true ;
    }

    /**
     * Get the character that is to be used for echoing. An echo character
     * is useful for text fields where user input should not be shown on the
     * screen, as in the case of a text field for entering a password.
     *
     * @return The echo character for this text field.
     */
    public char getEchoChar()
    {
        return m_cEchoChar ;
    }


    /**
     * Clear the echo character associated with this text field.  The text
     * field will display the natural / unmasked data.
     */
    public void clearEchoChar()
    {
        m_cEchoChar = 0 ;
        m_bEchoCharIsSet = false ;
        m_bEchoDelayIsSet = false;
        repaint() ;

    }


    /**
     * Indicate whether or not this text field has a character set for
     * echoing. An echo character is useful for text fields where user input
     * should not be shown on the screen, as in the case of a text field for
     * entering a password.
     *
     * @return True if this text field has a character set for echoing; false
     *         otherwise.
     */
    public boolean echoCharIsSet()
    {
        return m_bEchoCharIsSet ;
    }


    /**
     * Toggle the text field into or out of caps lock mode.  By default, the
     * text field component is not in caps lock mode and all characters are
     * entered in lower case. If shift lock mode is enabled, all character
     * are entered in upper case. Shift lock mode stays enabled until
     * specifically disabled.
     *
     * @param bShiftLock True to enable shift lock mode, or
     *        false to disable.
     */
    public synchronized void setCapsLock(boolean bCapsLock)
    {
        m_bCapsLock = bCapsLock ;
    }

    /**
     * make component go into IP Address entering mode
     */
    public synchronized void setNumericMode(boolean bNumMode)
    {
        m_bNumericMode = bNumMode;
    }

    /**
     * ask if component is in IP Address entering mode
     */
    public synchronized boolean getNumericMode()
    {
        return m_bNumericMode;
    }

    /**
     * Get the value of this text field component's caps lock mode.
     *
     * @return True if the field is in shift lock (all characters entered in
     *         uppercase) otherwise false (all characters entered in
     *         lowercase).
     */
    public boolean getCapsLock()
    {
        return m_bCapsLock ;

    }


    /**
     * Toggle the text field into or out of alphanumeric mode.  By default,
     * the text field component allows alphanumeric (both number and letters)
     * text entry.  Developers can turn off this feature and force purely
     * numeric text entry.
     *
     * @param bSet True to set alphanumeric mode, false to disable it.
     */
    public synchronized void setAlphanumericMode(boolean bSet)
    {
        // Move caret forward if turning off alphanumeric mode
        if (m_bAlphaNumericMode && !bSet)
            advanceCaret() ;

        m_bAlphaNumericMode = bSet ;
        m_iLastKey = -1 ;
        m_bCapsLock = false ;
        repaint() ;
    }


    /**
     * @return true if we are in alphanumeric mode or false for numeric mode
     */
    public boolean getAlphanumericMode()
    {
        return m_bAlphaNumericMode ;
    }


    /**
     * Add a text listener to this text field component. Text listeners
     * receive notification whenever the text within the text field component
     * is changed. If the specified listener has already been added, the add is
     * ignored with a silent error.
     *
     * @param listener The PTextListener that will receive update/change
     *        notifications.
     * @see org.sipfoundry.sipxphone.awt.event.PTextListener
     */
    public void addTextListener(PTextListener listener)
    {
        if (m_vTextListeners == null)
            m_vTextListeners = new Vector() ;

        if (!m_vTextListeners.contains(listener))
            m_vTextListeners.addElement(listener) ;
    }


    /**
     * Remove a text listener from this text field component.  If the
     * specified text listener is not listening, the request is ignored with
     * a silent error.
     *
     * @param listener The PTextListener that should stop receiving update/change
     *        notifications.
     * @see org.sipfoundry.sipxphone.awt.event.PTextListener
     */
    public void removeTextListener(PTextListener listener)
    {
        if (m_vTextListeners != null)
            m_vTextListeners.removeElement(listener) ;
    }


    /**
     * Add a focus listener to this text field component. Focus listeners
     * receive notification whenever this component gains or loses input
     * focus.  If the specified listener has already been added, the add is
     * ignored with a silent error.
     *
     * @param listener The PFocusListener that will receive focus change
     *        notifications.
     *
     * @see org.sipfoundry.sipxphone.awt.event.PFocusListener
     */
    public synchronized void addFocusListener(PFocusListener listener)
    {
        if (m_vFocusListeners == null)
            m_vFocusListeners = new Vector() ;

        if (!m_vFocusListeners.contains(listener))
            m_vFocusListeners.addElement(listener) ;
    }


    /**
     * Remove a focus listener from this text field component.  If the
     * specified focus listener is not listening, the request is ignored with
     * a silent error.
     *
     * @param listener The PFocusListenre that should stop receiving focus change
     *        notifications.
     * @see org.sipfoundry.sipxphone.awt.event.PFocusListener
     */
    public synchronized void removeFocusListener(PFocusListener listener)
    {
        if (m_vFocusListeners != null)
            m_vFocusListeners.removeElement(listener) ;
    }



    /**
     * @deprecated do not expose
     */
    public void paint(Graphics g)
    {
        String strPaint = "";           // String we will paint

        synchronized (m_objMutex) {
            Dimension dim = getSize() ;
            Rectangle rectBounds = new Rectangle(0, 0, dim.width, dim.height) ;

            if (((m_iLabelAlignment & PLabel.ALIGN_NORTH) > 0) || ((m_iLabelAlignment & PLabel.ALIGN_SOUTH) > 0)) {
                g.setFont(m_displayLabelFont) ;
            } else {
                g.setFont(m_displayFont) ;
            }

            // If we are using an echo char, create a phantom string

            boolean bCaretAdvance = false ;
            boolean bCaretBackspace = false ;
            if (m_iCaretPosition > m_iLastCaretPosition)
                bCaretAdvance = true ;
            if (m_iCaretPosition < m_iLastCaretPosition)
                bCaretBackspace = true ;

            /* If we're entering an alphanumeric password, display last char in
             * following conditions:
             * - the caret isn't advancing OR
             * - we're not moving forward because the timer has expired
             */
            if (m_bEchoDelayIsSet && ((bCaretAdvance==false) || (m_bTimerAdvanced==false)) && !bCaretBackspace) {
                strPaint = "" ;
                if (m_strText.length() >0) {
                    for (int i=0; i<(m_strText.length() - 1); i++)
                        strPaint += m_cEchoChar ;
                    strPaint += m_strText.substring(m_strText.length()-1) ;
                }
            // Otherwise if echo set display ALL echo characters
            } else if (m_bEchoCharIsSet) {
                strPaint = "" ;
                for (int i=0; i<m_strText.length(); i++)
                    strPaint += m_cEchoChar ;
            } else
                strPaint = m_strText ;

            // Reset internal data
            m_iLastCaretPosition = m_iCaretPosition ;
            m_bTimerAdvanced = false ;

            if (!m_bReadOnly)
                drawTextFrame(g, rectBounds) ;

            // Display the Label
            if (m_strLabel != null) {
                int xOffset = GUIUtils.calcXOffset(m_strLabel, g, rectBounds, m_iLabelAlignment) ;
                int yOffset = GUIUtils.calcYOffset(m_strLabel, g, rectBounds, m_iLabelAlignment) ;

                g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;

                if ((m_iLabelAlignment & PLabel.ALIGN_NORTH) == PLabel.ALIGN_NORTH)
                    g.drawString(m_strLabel, xOffset, yOffset+2) ;
                else
                    g.drawString(m_strLabel, xOffset, yOffset) ;
            }


            // Draw Text
            drawText(g, strPaint, m_iCaretPosition) ;
        }
    }


    /**
     * @deprecated do not expose
     */
    public void doLayout()
    {
        calculateDimensions() ;
        super.doLayout() ;
    }


    /**
     * @deprecated do not expose
     */
    public void repaint()
    {
        super.repaint() ;

//        Graphics g = getGraphics() ;
//        Dimension dimSize = getSize() ;

//        if ((g != null) && (dimSize.height > 0) && (dimSize.width > 0)) {
/*
            Image imgOffscreen = createImage(dimSize.width, dimSize.height) ;
            Graphics og = imgOffscreen.getGraphics() ;
            og.setClip(0, 0, dimSize.width, dimSize.height) ;
            paint(og) ;
            g.drawImage(imgOffscreen, 0, 0, this) ;
            og.dispose() ;
            g.dispose() ;
*/
//            super.paint(g) ;
//        }

    }

    /**
     * Determines if this component has input focus.
     *
     * @return true if this text field has input focus, otherwise false.
     */
    public boolean hasFocus()
    {
        return m_bHasFocus ;
    }


    /**
     * Sets input focus to this text field.
     *
     * @param bSet boolean true to set focus, otherwise false.
     */
    public synchronized void setFocus(boolean bSet)
    {
        // Ignore focus set if readonly or disabled
        if (bSet && (m_bReadOnly || !m_bEnabled)) {
            bSet = false ;
        }

        // Process the Focus Change
        if (bSet != m_bHasFocus) {
            m_bCycling = false ;
            m_bHasFocus = bSet ;
            fireFocusUpdate(bSet) ;
            repaint() ;
        }
    }


    /**
     * Set the read only attribute for this text field.  A read only text
     * field cannot receive input focus and is rendered with our the text
     * field frame.  All text fields are editable (not read only) by default
     * unless explictly set read only.
     *
     * @param bReadOnly true to set read only text field or false to allow
     *        editing.
     */
    public void setReadOnly(boolean bReadOnly)
    {
        m_bReadOnly = bReadOnly ;

        // Drop focus if we had it
        if (m_bReadOnly && hasFocus()) {
            setFocus(false) ;
        }
        repaint() ;
    }


    /**
     * Get the read only state of this text field.  A read only text field
     * cannot receive input focus and is rendered without a text frame.
     *
     * @return true if the text field is read only otherwise false.
     */
    public boolean isReadOnly()
    {
        return m_bReadOnly ;
    }


    /**
     * Set the enabled status of this text field.  A disabled text field is
     * cannot receive input focus and it's content text is displayed in gray.
     * All text fields are enabled by default unless explictly disabled.
     *
     * @param bEnable true to enable the component otherwise false to disable
     *        the component.
     */
    public void setEnabled(boolean bEnable)
    {
        m_bEnabled = bEnable ;

        // Drop focus if we had it
        if (!m_bEnabled && hasFocus()) {
            setFocus(false) ;
        }
    }


    /**
     * Get the enabled state of this text field.  A disabled text field cannot
     * receive input focus and is d


     A read only text field
     * cannot receive input focus and is rendered without a text frame.
     *
     * @return true if the text field is read only otherwise false.
     */
    public boolean isEnabled()
    {
        return m_bEnabled ;
    }


    /**
     * Set a label for this text field.  The label will be displayed using
     * the specified alignment relative to the text.  For example, an
     * alignment of (NORTH | WEST) will place the text above the text field
     * in the upper left.
     *
     * @param strText the text that will be displayed in this control
     * @param iAlignment where/how the text should be aligned
     *
     * @see #getLabelAlignment
     * @see #getLabel
     */
    public void setLabel(String strLabel, int iAlignment)
    {
        m_strLabel = strLabel ;
        m_iLabelAlignment = iAlignment ;

        if ((iAlignment != (PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST)) && (iAlignment != PLabel.ALIGN_WEST)) {
            System.out.println("WARNING: PTextField::setLabel *currently* only supports NORTHWEST and WEST alignments") ;
            m_iLabelAlignment = (PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST) ;
        }

        calculateDimensions() ;
        repaint() ;
    }


    /**
     * Get the label alignment for text field's label.  The alignment
     * specifies how the label is aligned relatively to the text field.
     *
     * @return the alignment id for the text field's label
     *
     * @see #setLabel
     * @see #getLabel
     */
    public int getLabelAlignment()
    {
        return m_iLabelAlignment ;
    }


    /**
     * get the user label set for this text field.  The user label is
     * displayed relative to the text field by some alignment factor.
     *
     * @return label alignment id
     *
     * @see #setLabel
     * @see #getLabelAlignment
     */
    public String getLabel()
    {
        return m_strLabel ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Fire an update/change notification to all listening text listeners.
     */
    private synchronized void fireFocusUpdate(boolean bHasFocus)
    {
        if (m_vFocusListeners != null) {
            PFocusEvent event = new PFocusEvent(this) ;

            int iLength = m_vFocusListeners.size() ;
            for (int i=(iLength-1); i>=0; i--) {
                PFocusListener listener = (PFocusListener) m_vFocusListeners.elementAt(i) ;
                try {
                    if (bHasFocus)
                        listener.focusGained(event) ;
                    else
                        listener.focusLost(event) ;
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


    /**
     * Fire an update/change notification to all listening text listeners.
     */
    private void fireTextUpdate()
    {
        if (m_vTextListeners != null) {
            PTextEvent event = new PTextEvent(this) ;
            Enumeration enum = m_vTextListeners.elements() ;
            while (enum.hasMoreElements()) {
                PTextListener listener = (PTextListener) enum.nextElement() ;
                if (listener != null) {
                    try {
                        listener.textValueChanged(event) ;
                    } catch (Throwable t) {
                        Shell.getInstance().showUnhandledException(t, true) ;
                    }
                }
            }
        }
    }


    /**
     * Fire an update/change notification to all listening text listeners.
     */
    private void fireCursorUpdate()
    {
        if (m_vTextListeners != null) {
            PTextEvent event = new PTextEvent(this) ;
            Enumeration enum = m_vTextListeners.elements() ;
            while (enum.hasMoreElements()) {
                PTextListener listener = (PTextListener) enum.nextElement() ;
                if (listener != null) {
                    try {
                        listener.caretPositionChanged(event) ;
                    } catch (Throwable t) {
                        Shell.getInstance().showUnhandledException(t, true) ;
                    }
                }
            }
        }
    }


    /**
     * insert a character at the caret position (and advanced the caret position one notch)
     */
    public void insertCharacter(char ch)
    {
        if (m_bCapsLock)
            ch = Character.toUpperCase(ch) ;

        synchronized (m_objMutex) {
            m_strText = m_strText.substring(0, m_iCaretPosition) + ch +  m_strText.substring(m_iCaretPosition) ;
            fireTextUpdate() ;

            if (!m_bAlphaNumericMode)
                advanceCaret() ;
            else
                repaint() ;
        }
    }


    /**
     * Replace the next character (or insert and set caret just before it)
     */
    private void replaceCharacter(char ch)
    {
        if (m_bCapsLock)
            ch = Character.toUpperCase(ch) ;

        synchronized (m_objMutex) {
            // Should we add a character to the end?
            if (m_iCaretPosition >= m_strText.length()) {
                m_strText += ch ;
            }
            else {
                byte b[] = m_strText.getBytes() ;
                b[m_iCaretPosition] = (byte) ch ;
                m_strText = new String(b) ;
            }
            fireTextUpdate() ;
            repaint() ;
        }
    }


    /**
     *
     */
    private void rotateAlphanumeric(int iRow)
    {
        m_iLastAlphaIndex = (m_iLastAlphaIndex + 1) % DEF_ALPHANUMERIC_KEYS[iRow].length ;
        replaceCharacter(DEF_ALPHANUMERIC_KEYS[iRow][m_iLastAlphaIndex]) ;
    }


    /**
     *
     */
    private void newAlphanumeric(int iRow, boolean bAdvanceCaret)
    {
        // Reset to index 1 (if possible) - first letter instead of digit
        m_iLastAlphaIndex = (1) % DEF_ALPHANUMERIC_KEYS[iRow].length ;

        if (bAdvanceCaret) {
            advanceCaret() ;
        }
        m_bCycling = true ;
        insertCharacter(DEF_ALPHANUMERIC_KEYS[iRow][m_iLastAlphaIndex]) ;

        // HACK: advance if only 1 char in list
        if (m_bAlphaNumericMode && (DEF_ALPHANUMERIC_KEYS[iRow].length == 1)) {
            m_bClearLastKey = true ;
            m_bCycling = false ;
            advanceCaret() ;
        }
    }


    /**
     *
     */
    private void drawText(Graphics g, String strPaintCandidate, int iCaretPosition)
    {
        if ((m_iTextFieldWidth <= 0) || (m_iTextFieldHeight <= 0))
            return ;

        if (m_bEnabled)
            g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
        else
            g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT_DISABLED)) ;

        FontMetrics fm = g.getFontMetrics() ;
        if (fm != null) {
            int iTextHeight = fm.getAscent() ;

            int iCharWidth = fm.charWidth('0') ;
            String strPaint = null ;

            // Make sure length isn't greater than string.  This could happen
            // if the string was replaced or deleted from the beginning.
            if (m_iTextFieldHScroll > strPaintCandidate.length())
            {
                m_iTextFieldHScroll = strPaintCandidate.length() ;
            }


            /*
             * Make sure caret is within view
             */

            // If the caret position is before the scroll, we need to move the
            // window to the left to match the caret postion
            strPaint = strPaintCandidate.substring(m_iTextFieldHScroll) ;
            if (iCaretPosition <= m_iTextFieldHScroll)
            {
                m_iTextFieldHScroll = Math.max(iCaretPosition - 1, 0) ;
                strPaint = strPaintCandidate.substring(m_iTextFieldHScroll) ;
            }

            // It is equally possible that the caret is to the right of our
            // window, we need to adjust.
            else if (fm.stringWidth(strPaint.substring(0, iCaretPosition-m_iTextFieldHScroll)) > ((m_iTextFieldWidth-4) - (iCharWidth))) {
                do {
                    m_iTextFieldHScroll++ ;
                    strPaint = strPaintCandidate.substring(m_iTextFieldHScroll) ;
                } while (fm.stringWidth(strPaint.substring(0, iCaretPosition-m_iTextFieldHScroll)) > ((m_iTextFieldWidth-4) - (iCharWidth))) ;
            }

            //calculate how many chars is half the control
            Dimension d = getSize();
            int iHalfCharLength = (d.width/iCharWidth)-((d.width/iCharWidth)/3);

            // only scroll back ten when we have 1 char left and the entire
            // length is greater than 1
            if (strPaint.length() == 1 && strPaintCandidate.length() > 1)
            {
                int iPaintLen = strPaintCandidate.length();

                if (iPaintLen < iHalfCharLength)
                {
                    //get the remainder
                    strPaint = strPaintCandidate.substring(0,iPaintLen);
                    m_iTextFieldHScroll = 0;
                }
                else
                {
                    //move back half
                    m_iTextFieldHScroll -= iHalfCharLength;
                    //and then get the next ten chars
                    if (m_iTextFieldHScroll < 0)
                        m_iTextFieldHScroll = 0;

                    int iAmountToWrite = m_iTextFieldHScroll+iHalfCharLength+1;

                    if (iAmountToWrite > strPaintCandidate.length())
                        iAmountToWrite = strPaintCandidate.length();

                    strPaint = strPaintCandidate.substring(m_iTextFieldHScroll,iAmountToWrite);
                }
            }


            /*
             * Make sure we don't run off the edge of the screen
             */
            while (fm.stringWidth(strPaint) > ((m_iTextFieldWidth-4)))
            {
                strPaint = strPaint.substring(0, strPaint.length()-1) ;
            }

            g.drawString(strPaint, m_iTextFieldXOffset, (m_iAlignYOffset + m_iTextFieldYOffset + iTextHeight) - 2) ;

            // Only draw caret if we have focus
            if (m_bHasFocus) {

                // Draw the caret:  We draw a horizontal caret for replacement mode and a vertical caret for insertion
                if (m_bAlphaNumericMode && m_bCycling) {
                    int iCaretStartLocation = 0 ;
                    int iCaretLength = 0 ;

                    // Start Position is easy
                    iCaretStartLocation = fm.stringWidth(strPaint.substring(0, iCaretPosition-m_iTextFieldHScroll)) ;
                    // End Position is a bit harder if we are at the last character
                    if (iCaretPosition < strPaint.length()) {
                        iCaretLength = fm.stringWidth(strPaint.substring(iCaretPosition-m_iTextFieldHScroll, iCaretPosition-m_iTextFieldHScroll+1)) ;
                    } else {
                        iCaretLength = iCharWidth ;    // use a random character since we don't have a average width handy
                    }

                    g.drawLine( iCaretStartLocation + m_iTextFieldXOffset - 1,
                            m_iAlignYOffset + m_iTextFieldYOffset + iTextHeight ,
                            iCaretStartLocation + m_iTextFieldXOffset + iCaretLength,
                            m_iAlignYOffset + m_iTextFieldYOffset + iTextHeight) ;
                } else {
                    // Numeric Mode / Insertion Mode will use a vertical caret
                    int iCaretLocation = fm.stringWidth(strPaint.substring(0, iCaretPosition-m_iTextFieldHScroll)) ;
                    g.drawLine(
                            m_iTextFieldXOffset + iCaretLocation - 1,
                            m_iAlignYOffset + m_iTextFieldYOffset ,
                            m_iTextFieldXOffset + iCaretLocation - 1,
                            m_iAlignYOffset + m_iTextFieldYOffset + iTextHeight) ;
                }
            }
        }
    }


    /**
     *
     */
    private void calculateDimensions()
    {
        Dimension dimSize = getSize() ;

        Graphics g = getGraphics() ;

        if (g != null) {
            /*
             * This text control automatically switches between fonts based
             * on the TextField/Label.  If a label is displayed above or below
             * the textfield then a smaller font is used.
             */

            if (((m_iLabelAlignment & PLabel.ALIGN_NORTH) > 0) || ((m_iLabelAlignment & PLabel.ALIGN_SOUTH) > 0)) {
                g.setFont(m_displayLabelFont) ;
            } else {
                g.setFont(m_displayFont) ;
            }

            FontMetrics fm = g.getFontMetrics() ;
            if (fm != null) {
                int iCharWidth = fm.charWidth('O') ;

                // Calc width
                if ((m_iTextFieldLength == -1) || m_bAutoTextFieldLength) {
                    // Calculate max
                    m_iTextFieldLength = (dimSize.width-8) / iCharWidth ;
                    m_iTextFieldWidth = m_iTextFieldLength * iCharWidth + 4 ;
                } else {
                    m_iTextFieldWidth = (m_iTextFieldLength * iCharWidth) ;
                    // Use passed, but truncated if too large
                    while (m_iTextFieldWidth > (dimSize.width-8)) {
                        m_iTextFieldLength-- ;
                        m_iTextFieldWidth = (m_iTextFieldLength * iCharWidth) + 4 ;
                    }
                }

                // Calc Height (ascent + descent)
                m_iTextFieldHeight = fm.getAscent() + fm.getDescent() ;

                m_iAlignXOffset = 0 ;
                m_iAlignYOffset = 0 ;

                // Now calculate drawing offsets
                m_iTextFieldXOffset = 4 ;

                if ((m_iLabelAlignment == PLabel.ALIGN_WEST)  && (m_strLabel != null)) {
                    int iLabelWidth = fm.stringWidth(m_strLabel) ;

                    m_iTextFieldXOffset += iLabelWidth ;
                    m_iTextFieldWidth -= iLabelWidth ;

                }

                // Adjust for label alignment
                if ((m_iLabelAlignment & PLabel.ALIGN_NORTH) == PLabel.ALIGN_NORTH) {
                    // Label is on top, align text field south
                    m_iTextFieldYOffset = (dimSize.height - m_iTextFieldHeight)  ;

                } else if ((m_iLabelAlignment & PLabel.ALIGN_SOUTH) == PLabel.ALIGN_SOUTH) {
                    // Label is on bottom, align text field north
                    m_iTextFieldYOffset = 0 ;
                } else {
                    // Label is centered, center text field
                    m_iTextFieldYOffset = ((dimSize.height - m_iTextFieldHeight) / 2) ;
                }
            }
        }
    }


    /**
     *
     */
    private void drawTextFrame(Graphics g, Rectangle rect)
    {
        Color colorOld = g.getColor() ;

        // Fill entire area
        g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND)) ;
        g.fillRect(rect.x, rect.y + m_iAlignYOffset, rect.width, rect.height) ;

        if (m_bHasFocus)
        {
            g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND_FOCUS)) ;
            g.fillRect(m_iTextFieldXOffset-1, m_iAlignYOffset + m_iTextFieldYOffset-1, m_iTextFieldWidth, m_iTextFieldHeight) ;
        }

        // Draw borders
        g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_BORDER_DARK)) ;

        //    __
        //   x__|
        //
        g.drawLine( m_iTextFieldXOffset-2,
                    m_iAlignYOffset + m_iTextFieldYOffset-2,
                    m_iTextFieldXOffset-2,
                    m_iAlignYOffset + m_iTextFieldYOffset-2 + m_iTextFieldHeight) ;

        //    xx
        //   |__|
        //
        g.drawLine( m_iTextFieldXOffset-2,
                    m_iAlignYOffset + m_iTextFieldYOffset-2,
                    m_iTextFieldXOffset-2 + m_iTextFieldWidth,
                    m_iAlignYOffset + m_iTextFieldYOffset-2) ;

        g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_BORDER_LIGHT)) ;

        //    __
        //   |  |
        //    xx
        g.drawLine( m_iTextFieldXOffset-2+1,
                    m_iAlignYOffset + m_iTextFieldYOffset-2+m_iTextFieldHeight,
                    m_iTextFieldXOffset-2+m_iTextFieldWidth,
                    m_iAlignYOffset + m_iTextFieldYOffset-2+m_iTextFieldHeight) ;

        //    __
        //   |__x
        //
        g.drawLine( m_iTextFieldXOffset-2+m_iTextFieldWidth,
                    m_iAlignYOffset + m_iTextFieldYOffset-2+1,
                    m_iTextFieldXOffset-2+m_iTextFieldWidth,
                    m_iAlignYOffset + m_iTextFieldYOffset-2+m_iTextFieldHeight) ;

        g.setColor(colorOld) ;
    }

    protected void onClipboardPaste()
    {
        try
        {
            // non-destructive paste, insert clipboard where caret position is
            StringBuffer sb = new StringBuffer().append(getText());
            Clipboard clip = Toolkit.getDefaultToolkit().getSystemClipboard();
            String pasteText = (String)(clip.getContents(this).getTransferData(DataFlavor.stringFlavor));
            pasteText = cleanString(pasteText);
            sb.insert(getCaretPosition(), pasteText);
            setText(sb.toString());
        }
        catch(Exception e)
        {
            System.out.println("****** Could not paste from clipboard!");
        }
    }

    /**
     * Strip out control chars and non-digits if in numeric mode
     */
    private String cleanString(String s)
    {
        if (s == null)
            return s;

        StringBuffer sb = new StringBuffer(s.length());
        for (int i = 0; i < s.length(); i++)
        {
            char c = s.charAt(i);
            if (c >= ' ')
            {
                if (!m_bNumericMode || Character.isDigit(c))
                {
                    sb.append(c);
                }
            }
        }

        return sb.toString();
    }


    protected void onClipboardCopy()
    {
        try
        {
            Clipboard clip = Toolkit.getDefaultToolkit().getSystemClipboard();
           clip.setContents(new StringSelection(m_strText),new StringSelection(m_strText));
        }
        catch(Exception e)
        {
            System.out.println("****** Could not copy from text PTextField!");
        }
    }

    /**
     * Button Down Handler
     */
    protected void onButtonDown(PButtonEvent event)
    {
        boolean bJumpedKey = ((m_iLastKey != -1) && (m_iLastKey != event.getButtonID())) ;
        boolean bNewKey = (m_iLastKey == -1) || (m_iLastKey != event.getButtonID()) ;

        // Are we now in insert or replace/rotate mode?
        m_bCycling = m_bAlphaNumericMode ;


// System.out.println("PTextField OnButtonDown: Got ButtonId = " + event.getButtonID());
        if (event.getEventType() == event.KEY_DOWN)
        {
            if (event.getButtonID() == CTRL_C)
            {
                onClipboardCopy();
            }
            else if (event.getButtonID() == CTRL_V)
            {
                onClipboardPaste();
            }
            else if (event.getButtonID() == 8) //backspace
            {
// System.out.println("deleting character");
                m_bCycling = false;
                backspace();
            }
            else
            if (event.getButtonID() == PButtonEvent.BID_HOME) //HOME KEY
            {
                Home();

            }
            else
            if (event.getButtonID() == PButtonEvent.BID_END) //END KEY
            {
                End();
            }
            else
            {
                if (!m_bAlphaNumericMode &&
                    !Character.isDigit((char)event.getButtonID()) &&
                    (char)event.getButtonID() != '*' &&
                    (char)event.getButtonID() != '#' &&
                    (char)event.getButtonID() != '.'
                    )
                {
                    event.consume();
                    return; //dont process letters on a non alph text field
                }

                insertCharacter((char)event.getButtonID()) ;

                if (m_bAlphaNumericMode)
                    advanceCaret() ;
            }
        }
        else
        {
            switch (event.getButtonID())
            {
                case PButtonEvent.BID_0:

                    if (m_bNumericMode)
                    {
                        insertCharacter('0') ;
                        advanceCaret() ;
                    }
                    else
                    if (m_bAlphaNumericMode)
                    {
                        if (bNewKey)
                            newAlphanumeric(0, bJumpedKey) ;
                        else
                            rotateAlphanumeric(0) ;
                    }
                    else
                    {
                        insertCharacter('0') ;
                    }
                    break ;
                case PButtonEvent.BID_1:
                    if (m_bNumericMode)
                    {
                        insertCharacter('1') ;
                        advanceCaret() ;
                    }
                    else
                    if (m_bAlphaNumericMode)
                    {
                        if (bNewKey)
                            newAlphanumeric(1, bJumpedKey) ;
                        else
                            rotateAlphanumeric(1) ;
                    }
                    else
                    {
                        insertCharacter('1') ;
                    }
                    break ;
                case PButtonEvent.BID_2:
                    if (m_bNumericMode)
                    {
                        insertCharacter('2') ;
                        advanceCaret() ;
                    }
                    else
                    if (m_bAlphaNumericMode)
                    {
                        if (bNewKey)
                            newAlphanumeric(2, bJumpedKey) ;
                        else
                            rotateAlphanumeric(2) ;
                    }
                    else
                    {
                        insertCharacter('2') ;
                    }
                    break ;
                case PButtonEvent.BID_3:
                    if (m_bNumericMode)
                    {
                        insertCharacter('3') ;
                        advanceCaret() ;
                    }
                    else
                    if (m_bAlphaNumericMode)
                    {
                        if (bNewKey)
                            newAlphanumeric(3, bJumpedKey) ;
                        else
                            rotateAlphanumeric(3) ;
                    }
                    else
                    {
                        insertCharacter('3') ;
                    }
                    break ;
                case PButtonEvent.BID_4:
                    if (m_bNumericMode)
                    {
                        insertCharacter('4') ;
                        advanceCaret() ;
                    }
                    else
                    if (m_bAlphaNumericMode)
                    {
                        if (bNewKey)
                            newAlphanumeric(4, bJumpedKey) ;
                        else
                            rotateAlphanumeric(4) ;
                    }
                    else
                    {
                        insertCharacter('4') ;
                    }
                    break ;
                case PButtonEvent.BID_5:
                    if (m_bNumericMode)
                    {
                        insertCharacter('5') ;
                        advanceCaret() ;
                    }
                    else
                    if (m_bAlphaNumericMode)
                    {
                        if (bNewKey)
                            newAlphanumeric(5, bJumpedKey) ;
                        else
                            rotateAlphanumeric(5) ;
                    }
                    else
                    {
                        insertCharacter('5') ;
                    }
                    break ;
                case PButtonEvent.BID_6:
                    if (m_bNumericMode)
                    {
                        insertCharacter('6') ;
                        advanceCaret() ;
                    }
                    else
                    if (m_bAlphaNumericMode)
                    {
                        if (bNewKey)
                            newAlphanumeric(6, bJumpedKey) ;
                        else
                            rotateAlphanumeric(6) ;
                    }
                    else
                    {
                        insertCharacter('6') ;
                    }
                    break ;
                case PButtonEvent.BID_7:
                    if (m_bNumericMode)
                    {
                        insertCharacter('7') ;
                        advanceCaret() ;
                    }
                    else
                    if (m_bAlphaNumericMode)
                    {
                        if (bNewKey)
                            newAlphanumeric(7, bJumpedKey) ;
                        else
                            rotateAlphanumeric(7) ;
                    }
                    else
                    {
                        insertCharacter('7') ;
                    }
                    break ;
                case PButtonEvent.BID_8:
                    if (m_bNumericMode)
                    {
                        insertCharacter('8') ;
                        advanceCaret() ;
                    }
                    else
                    if (m_bAlphaNumericMode)
                    {
                        if (bNewKey)
                            newAlphanumeric(8, bJumpedKey) ;
                        else
                            rotateAlphanumeric(8) ;
                    }
                    else
                    {
                        insertCharacter('8') ;
                    }
                    break ;
                case PButtonEvent.BID_9:
                    if (m_bNumericMode)
                    {
                        insertCharacter('9') ;
                        advanceCaret() ;
                    }
                    else
                    if (m_bAlphaNumericMode)
                    {
                        if (bNewKey)
                            newAlphanumeric(9, bJumpedKey) ;
                        else
                            rotateAlphanumeric(9) ;
                    }
                    else
                    {
                        insertCharacter('9') ;
                    }
                    break ;
                case PButtonEvent.BID_STAR:

                    if (m_bNumericMode)
                    {
                        insertCharacter('.') ;
                        advanceCaret() ;
                    }
                    else
                    if (m_bAlphaNumericMode)
                    {
                        if (bNewKey)
                            newAlphanumeric(10, bJumpedKey) ;
                        else
                            rotateAlphanumeric(10) ;
                    }
                    else
                    {
                        insertCharacter('*') ;
                    }
                    break ;
                case PButtonEvent.BID_POUND:
                    if (m_bAlphaNumericMode)
                    {
                        if (bNewKey)
                            newAlphanumeric(11, bJumpedKey) ;
                        else
                            rotateAlphanumeric(11) ;
                    }
                    else
                    {
                        insertCharacter('#') ;
                    }
                    break ;
            }
        }

        // Store 'last key' if we are in alpha numeric mode
        if (m_bAlphaNumericMode)
        {
            if (m_bClearLastKey)
            {
                m_iLastKey = -1 ;
                m_bClearLastKey = false ;
            }
            else
                m_iLastKey = event.getButtonID() ;
        }
    }



    /**
     * Button Up Handler
     */
    protected void onButtonUp(PButtonEvent event)
    {
        if (m_bCycling)
            Timer.getInstance().resetTimer(Settings.getInt("ALPHANUMERIC_TIMEOUT_LENGTH", 1500), m_timeoutHandler, null) ;
    }




//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////
    /**
     * Handles the firing of the alphanumeric timer.  Upon firing, this action
     * listener simply advances the caret position.
     */
    private class icAlphanumericTimeout implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            m_bTimerAdvanced = true ;
            advanceCaret() ;
        }
    }


    /**
     * Listens for button presses and hands them off to the onButton<Up|Down>
     * handler.
     */
    private class icButtonListener implements PButtonListener
    {
        /**
         * The specified button has been press downwards
         */
        public void buttonDown(PButtonEvent event)
        {
            if (m_bHasFocus)
                onButtonDown(event) ;
        }


        /**
         * The specified button has been released
         */
        public void buttonUp(PButtonEvent event)
        {
            if (m_bHasFocus)
                onButtonUp(event) ;
        }


        /** The specified button is being held down */
        public void buttonRepeat(PButtonEvent event) { }
    }
}
