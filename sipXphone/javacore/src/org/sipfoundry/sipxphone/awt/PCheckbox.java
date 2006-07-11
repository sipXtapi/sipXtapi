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
import java.util.Vector ;
import java.util.Enumeration ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.service.* ;

/**
 * A check box is a graphical component that can be either "on" (true) or
 * "off" (false).  Clicking the screen display button that is associated
 * with a check box changes its state from "on" to "off" or from "off" to "on".
 * <p>
 * In this release, the check box label is always left justified, and the
 * graphical check box is always right justified.
 */
public class PCheckbox extends PComponent implements PFocusTraversable
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

/////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** label of this check box */
    private String m_strLabel ;
    /** check box group (if specified) */
    private PCheckboxGroup m_group ;
    /** list if data item listeners */
    private Vector m_listeners ;
    /** hint displayed when a user presses and holds down the associated
        screen button */
    private String m_strHint ;
    /** button id bound to this check box */
    private int m_iAssociatedButtonId ;
    /** is the associated button currently held down? */
    private boolean m_bButtonDown ;

    private static Image s_imgSelectedCheck ;
    private static Image s_imgUnselectedCheck ;
    private static Image s_imgSelectedRadio ;
    private static Image s_imgUnselectedRadio ;

    private boolean m_bHasFocus ;
    private Vector  m_vFocusListeners ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Creates an unlabeled check box.  The state of this check box defaults
     * to "off" and this check box is not part of a check box group. No hint
     * will be displayed when the user presses down and holds the associated
     * screen display button.
     */
    public PCheckbox()
    {
        this(null) ;
    }


    /**
     * Creates a check box with the specified label. By default, the state of
     * this check box is "off" and it is not part of a check box group.
     * If a user presses down and holds the screen display button that is
     * associated with a check box, no hint displays.
     */
    public PCheckbox(String strLabel)
    {
        this (strLabel, false) ;
    }


    /**
     * Creates a check box with the specified label and state.  This check box
     * is not part of a check box group. If a user presses down and holds
     * the screen display button that is associated with a check box,
     * no hint displays.
     */
    public PCheckbox(String strLabel, boolean bState)
    {
        this(strLabel, bState, null) ;
    }


    /**
     * Creates a check box with the specified label, state, and check box group.
     * If a group is specified, only one check box in that group can be "on" or
     * checked at a given time; the check boxes in a group act like radio buttons
     * and are mutually exclusive.
     * <p>
     * One check box in each group must have a state of "on": you can set the state
     * of a new check box to "off" if another check box is later added to
     * the group with an "on" state.
     * <p>
     * If a user presses and holds down the screen display button that is
     * associated with a check box, no hint displays.
     * @see org.sipfoundry.sipxphone.awt.PCheckboxGroup
     */
    public PCheckbox(String strLabel, boolean bState, PCheckboxGroup group)
    {
        this(strLabel, bState, group, null) ;
    }


    /**
     * Creates a check box with the specified label, state, check box group,
     * and hint. If a group is specified, only one check box in that group can be
     * "on" or checked at a given time; the check boxes in a group are mutually
     * exclusive.
     * <p>
     * One check box in each group must have a state of "on": you can set the state
     * of a new check box to "off" if another check box is later added to
     * the group with an "on" state.
     * @see org.sipfoundry.sipxphone.awt.PCheckboxGroup
     */
    public PCheckbox(String strLabel, boolean bState, PCheckboxGroup group, String strHint)
    {
        setLabel(strLabel) ;
        setCheckboxGroup(group) ;
        setState(bState) ;
        setHint(strHint) ;
        setAssociatedButton(-1) ;
        m_bButtonDown = false ;
        m_bHasFocus = false ;
        m_vFocusListeners = null ;

        addButtonListener(new icButtonWatcher()) ;

        // Only load check box images if needed
        if (        (s_imgUnselectedCheck == null) ||
                    (s_imgSelectedRadio == null) ||
                    (s_imgSelectedCheck == null) ||
                    (s_imgSelectedRadio == null)) {
            s_imgSelectedCheck = getImage("imgCheckboxSelectedCheck") ;
            s_imgUnselectedCheck = getImage("imgCheckboxUnselectedCheck") ;
            s_imgSelectedRadio = getImage("imgCheckboxSelectedRadio") ;
            s_imgUnselectedRadio = getImage("imgCheckboxUnselectedRadio") ;
        }

        setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT)) ;
        enableDoubleBuffering(true) ;
    }



//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Sets the label of this check box to the specified string.
     *
     * @param strLabel A new string label or null for no label.
     */
    public void setLabel(String strLabel)
    {
        m_strLabel = strLabel ;

        if (isVisible())
            repaint() ;
    }


    /**
     * Gets the check box's label.
     *
     * @return The label of this check box or null if no label was set.
     */
    public String getLabel()
    {
        return m_strLabel ;
    }


    /**
     * Sets the state of this check box to the specified bState.  A value
     * of true indicates an "on" or checked state and a value of false
     * indicates an "off" or unchecked state.
     *
     * @param bState The new state of the check box.
     */
    public void setState(boolean bState)
    {
        boolean bWasSelected = (m_group.getSelectedCheckbox() == this) ;

        if (bState) {
            m_group.setSelectedCheckbox(this) ;
        } else {
            // Only a group of 1 allows you to uncheck
            if (m_group.getSize() == 1) {
                if (bWasSelected)
                    m_group.setSelectedCheckbox(null) ;
            } else {
                bState = true ;
            }
        }

        if (bWasSelected != bState)
            fireStateChange(bState) ;
    }


    /**
     * Gets the state of this check box.  A value of true indicates an "on" or
     * checked state and a value of false indicates an "off" or unchecked
     * state.
     *
     * @return Boolean representing this check box's state.
     */
    public boolean getState()
    {
        return (m_group.getSelectedCheckbox() == this) ;
    }

    /**
     * Is this a radio button or a regular check box?
     * @return true if its a radio button, false otherwise
     * @deprecated DO NOT EXPOSE YET
     */
    public boolean isRadioButton(){
        boolean bRet = false;
        if( (m_group != null) && (m_group.getSize() > 1) )
            bRet = true;
        return bRet;
    }

    /**
     * Sets the group for this check box. If a check box is added to a group,
     * each check box acts like a mutually exclusive radio button where only
     * one check box can be "on" or checked at any time. Any group
     * previously set for the check box is automatically removed.
     *
     * @param New checkbox group or null to remove this checkbox from any group.
     * @see org.sipfoundry.sipxphone.awt.PCheckboxGroup
     */
    public void setCheckboxGroup(PCheckboxGroup group)
    {
        // Detach old
        if (m_group != null) {
            m_group.removeCheckbox(this) ;
            m_group = null ;
        }

        // Attach new
        if (group == null) {
            group = new PCheckboxGroup() ;
        }
        m_group = group ;
        m_group.addCheckbox(this) ;
    }


    /**
     * Gets the check box's group.
     *
     * @return The check box's group or null if the check box is not part of
     *         any group.
     */
    public PCheckboxGroup getCheckboxGroup()
    {
        return m_group ;
    }


    /**
     * Sets the hint text to display when a user presses and holds down the
     * screen display button associated with this check box. The hint text
     * is centered and wrapped in the hint frame automatically.  Developers
     * can explictly force a line break by adding a &quot;\n&quot; into their text.
     *
     * @param strHint New hint text or null to remove the hint text.
     */
    public void setHint(String strHint)
    {
        m_strHint = strHint ;
    }


    /**
     * Gets the hint that displays when a user presses and holds down the
     * screen display button that is associated with the check box.
     *
     * @return Hint text.
     */
    public String getHint()
    {
        return m_strHint ;
    }


    /**
     * Adds the specified item listener to this check box to receive
     * notification when this check box's state is changed from "on" to "off"
     * or from "off" on "on".  If the specified item listener is already
     * listening to this check box, it is ignored.
     *
     * @param listener New item listener.
     */
    public void addItemListener(PItemListener listener)
    {
        if (m_listeners == null)
            m_listeners = new Vector() ;

        if (!m_listeners.contains(listener)) {
            m_listeners.addElement(listener) ;
        }
    }


    /**
     * Removes the specified item listener from this check box.  If the
     * specified item listener is not listening to this check box then
     * the remove is silently ignored.
     *
     * @param listener Item listener that is no longer interested in receiving
     *        state change notifications.
     */
    public void removeItemListener(PItemListener listener)
    {
        if (m_listeners != null)
            m_listeners.removeElement(listener) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Sets the button id that is associated with or leased to this component.
     *
     * @param iButtonId Button that the component should listen for or -1 to
     *        disassociate this checkbox from all screen display buttons.
     */
    protected void setAssociatedButton(int iButtonId)
    {
        m_iAssociatedButtonId = iButtonId ;
    }


    /**
     * The actual art of drawing
     *
     * @deprecated This should not be exposed
     */
    public void paint(Graphics g)
    {
        Dimension dimSize = getSize() ;
        Rectangle rectLabel = new Rectangle(0, 0, dimSize.width-16, dimSize.height) ;
        Rectangle rectArea = new Rectangle(0, 0, dimSize.width, dimSize.height) ;


        if (m_strLabel != null) {
            Color colorOld = g.getColor() ;
            g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;



            // Draw Label Area
            GUIUtils.drawWrappingString(g, m_strLabel, rectLabel, GUIUtils.ALIGN_WEST | GUIUtils.ALIGN_CENTER) ;

            // Draw State Area
            Image imgState = null ;

            if (getState())
                imgState = (m_group.getSize() == 1) ? s_imgSelectedCheck : s_imgSelectedRadio ;
            else
                imgState = (m_group.getSize() == 1) ? s_imgUnselectedCheck : s_imgUnselectedRadio ;

            g.drawImage(imgState,
                        GUIUtils.calcXImageOffset(imgState, rectArea, GUIUtils.ALIGN_EAST),
                        GUIUtils.calcYImageOffset(imgState, rectArea, GUIUtils.ALIGN_CENTER),
                        this) ;

            g.setColor(colorOld) ;

        }
    }

    /**
     * Set the focus state of this a traversable component.  A component
     * is required to have focus before receiving keyboard and/or button
     * events.
     *
     * @param bSet boolean true to set focus otherwise false to give it up.
     */
    public void setFocus(boolean bSet)
    {

        if (m_bHasFocus != bSet) {
            PFocusEvent event = new PFocusEvent(this) ;

            m_bHasFocus = bSet ;
            Enumeration enumListeners = m_vFocusListeners.elements() ;
            while (enumListeners.hasMoreElements()) {
                PFocusListener listener = (PFocusListener) enumListeners.nextElement() ;
                if (listener != null) {
                    if (bSet) {
                        listener.focusGained(event) ;
                    } else {
                        listener.focusLost(event) ;
                    }
                }
            }
        }
    }


    /**
     * Add a focus listener to this element. Focus listeners receive
     * notification whenever this component gains or loses input focus.
     * If the specified listener has already been added, the add should
     * be ignored.
     *
     * @param listener The PFocusListener that will receive focus change
     *        notifications.
     *
     * @see org.sipfoundry.sipxphone.awt.event.PFocusListener
     */
    public void addFocusListener(PFocusListener listener)
    {
        if (m_vFocusListeners == null)
            m_vFocusListeners = new Vector() ;

        if (!m_vFocusListeners.contains(listener)) {
            m_vFocusListeners.addElement(listener) ;
        }
    }


    /**
     * Remove a focus listener from this element.  If the specified focus
     * listener is not listening, the request should be ignored.
     *
     * @param listener The PFocusListenre that should stop receiving focus change
     *        notifications.
     *
     * @see org.sipfoundry.sipxphone.awt.event.PFocusListener
     */
    public void removeFocusListener(PFocusListener listener)
    {
        if (m_vFocusListeners != null)
            m_vFocusListeners.removeElement(listener) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Fires off a state change event to all interested parties.
     */
    private void fireStateChange(boolean bState)
    {
        if (m_listeners != null) {
            PItemEvent event = new PItemEvent(this, this, bState ? PItemEvent.SELECTED : PItemEvent.DESELECTED) ;

            Enumeration enumListeners = m_listeners.elements() ;
            while (enumListeners.hasMoreElements()) {
                PItemListener listener = (PItemListener) enumListeners.nextElement() ;
                if (listener != null) {
                    try {
                        listener.itemStateChanged(event) ;
                    } catch (Throwable t) {
                        Shell.getInstance().showUnhandledException(t, true) ;
                    }
                }
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner class
////
    /**
     * The button watcher is responsible for listening to button event and
     * managing hints and state changes.
     */
    private class icButtonWatcher implements PButtonListener, PActionListener
    {
        /**
         * The specified button has been press downwards
         */
        public void buttonDown(PButtonEvent event)
        {
            if (m_bButtonDown)
                return ;

            if (event.getButtonID() == m_iAssociatedButtonId) {

                if (!m_bHasFocus) {
                    setFocus(true) ;
                }

                m_bButtonDown = true ;
                Timer.getInstance().resetTimer(Settings.getInt("TOOLTIP_TIMEOUT_LENGTH", 1000), this, null) ;
                event.consume() ;
            }
        }


        /**
         * The specified button has been released
         */
        public void buttonUp(PButtonEvent event)
        {
            Timer.getInstance().removeTimers(this) ;

            if (m_bButtonDown) {
                m_bButtonDown = false ;
            }

            if (event.getButtonID() == m_iAssociatedButtonId) {

                if (!m_bHasFocus) {
                    setFocus(true) ;
                }

                setState(!getState()) ;
                event.consume() ;
            }
        }


        /** The specified button is being held down */
        public void buttonRepeat(PButtonEvent event) { }


        /**
         *
         */
        public void actionEvent(PActionEvent event)
        {
            if ((m_strHint != null) && (m_strHint.length() > 0)) {
                if (m_bButtonDown) {
                    Shell.getInstance().displayHint(m_strHint) ;
                    m_bButtonDown = false ;
                }
            }
        }
    }
}
