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

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;

import java.awt.* ;
import java.util.Vector ;
import java.util.Enumeration ;

import org.sipfoundry.sipxphone.service. * ;
import org.sipfoundry.sipxphone.sys. * ;


/**
 * The PBottomButtonBar is a horizontal button bar located on the bottom of
 * the form.  It displays up to three different actions or menu items represented by
 * images or text, and fires off PActionEvents whenever a button is pressed.
 *
 * @see PActionEvent
 * @author Robert J. Andreasen, Jr.
 */
public class PBottomButtonBar extends PContainer
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Button location constant: [ B1 ] [    ] [    ] */
    public static final int B1      = 0 ;
    /** Button location constant: [    ] [ B2 ] [    ] */
    public static final int B2      = 1 ;
    /** Button location constant: [    ] [    ] [ B3 ] */
    public static final int B3      = 2 ;

    private static final int MAX_BUTTONS = 3 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected PLabel  m_labels[] ;          // Labels for the button bar
    protected String  m_actionCommands[] ;  // action command for each button
    protected String  m_hints[] ;           // Hints for each button
    protected boolean m_bEnabled[] ;        // Is the label enabled?
    protected Vector  m_vListeners[];        // list of PActionListeners awaiting action notification for each button
    protected Vector  m_vGlobalListener = new Vector();        // list of PActionListeners awaiting action notification for all buttons
    protected int     m_iButtonDown = -1 ;  // Is a button being held down? (hints)

//OLD Vector    protected Vector m_vListeners = new Vector() ;

    /** handler for the tooltip implementation */
    protected icTooltipHandler  m_tooltipHandler = new icTooltipHandler() ;


//////////////////////////////////////////////////////////////////////////////
// Constructors
////
    /**
     * Default, no argument Constructor.
     */
    public PBottomButtonBar()
    {
        super() ;

        /* intialize all vectors */
        m_vListeners = new Vector[MAX_BUTTONS];
        for (int loop = 0;loop < MAX_BUTTONS;loop++)
            m_vListeners[loop] = new Vector();

        m_labels = new PLabel[MAX_BUTTONS] ;
        m_actionCommands = new String[MAX_BUTTONS] ;
        m_hints = new String[MAX_BUTTONS] ;
        m_bEnabled = new boolean[MAX_BUTTONS] ;

        addButtonListener(new icButtonListener()) ;
        setLayout(null) ;
        setOpaque(false) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Set the button information at the specified position.
     *
     * @param iPosition Position of the button.  The constants B1, B2, and B3
     *        should be used.
     * @param itemData {@link PActionItem} containing a label, hint text, and,
     *        optionally, an action listener and action command.
     *
     * @exception IllegalArgumentException Thrown if the itemData is null or
     *            iPosition is invalid.
     */
    public void setItem(int iPosition, PActionItem itemData)
        throws IllegalArgumentException
    {
        if (itemData == null) {
            throw new IllegalArgumentException("A non null PActionItem is required") ;
        }
        // Make sure the position is valid
        if ((iPosition < B1) || (iPosition > B3)) {
            throw new IllegalStateException("invalid position index") ;
        }

        setItem( iPosition,
             itemData.getLabel(),
             (itemData.getActionCommand() == null) ? "ACTION" : itemData.getActionCommand(),
             itemData.getHint() ) ;

        if (!m_vListeners[iPosition].contains(itemData.getActionListener()))
            m_vListeners[iPosition].addElement(itemData.getActionListener()) ;

        enableByAction(itemData.isEnabled(), itemData.getActionCommand()) ;
    }


    /**
     * Get the bottom button bar item at the specified index position.
     *
     * @param iPosition The position (B1, B2, or B3) of the item that you
     *        wish to retrieve.
     *
     * @return The item at the requested position or null if no item has been
     *         set.
     *
     * @exception IllegalStateException Thrown
     */
    public PActionItem getItem(int iPosition)
    {
        PActionItem item = null ;

        // Make sure the position is valid
        if ((iPosition < B1) || (iPosition > B3)) {
            throw new IllegalStateException("invalid position index") ;
        }

        if (m_labels[iPosition] != null) {
            item = new PActionItem(m_labels[iPosition], m_hints[iPosition], null, m_actionCommands[iPosition]) ;
        }
        return item ;
    }


    /**
     * Set the button information at the specified position.
     *
     * @param iPosition Position of the button.  The constants B1, B2, and B3
     *        should be used.
     * @param label Text or graphical representation of the action.
     * @param strActionCommand Action command to fire to listeners.
     * @param strHint Hint text to display when a button is held down. To include
     *        a title as well as text, use the format <hint title>|<hint text>.
     *
     * @exception IllegalArgumentException Thrown if the itemData is null or
     *            iPosition is invalid.
     */
    public void setItem(    int     iPosition,
                            PLabel  label,
                            String  strActionCommand,
                            String  strHint)
        throws IllegalArgumentException
    {
        // Make sure the position is valid
        if ((iPosition < B1) || (iPosition > B3)) {
            throw new IllegalArgumentException("invalid position index") ;
        }

        clearItem(iPosition) ;

        m_labels[iPosition] = label.makeClone() ;
        m_actionCommands[iPosition] = (strActionCommand == null) ? "ACTION" : strActionCommand ;
        m_hints[iPosition] = strHint ;
        m_bEnabled[iPosition] = true ;

        add(m_labels[iPosition]) ;

        m_labels[iPosition].setBounds(getComponentLocationAt(iPosition)) ;
        m_labels[iPosition].repaint() ;
    }


    /**
     * @deprecated Do not expose yet
     */
    public void enableByAction(boolean bEnable, String strAction)
    {
        for (int i=0;i<MAX_BUTTONS; i++) {
            if ((m_actionCommands[i] != null) && m_actionCommands[i].equals(strAction)) {
                if (m_bEnabled[i] != bEnable) {
                    m_bEnabled[i] = bEnable ;
                    if (bEnable) {
                        if (!containsComponent(m_labels[i])) {
                            m_labels[i].setBounds(getComponentLocationAt(i)) ;
                            add(m_labels[i]) ;

                            m_labels[i].repaint() ;
                        }
                    } else {
                        if (containsComponent(m_labels[i])) {
                            remove(m_labels[i]) ;

                            Rectangle rectPosition = getComponentLocationAt(i) ;
                            repaint(rectPosition.x, rectPosition.y, rectPosition.width, rectPosition.height) ;
                        }
                    }
                }
            }
        }
    }


    /**
     * Clear the button at the specified position.
     *
     * @param iPosition Position of the button.  The constants B1, B2, and B3
     *        should be used.
     *
     * @exception IllegalArgumentException Thrown if the itemData is null or
     *            iPosition is invalid.
     */
    public void clearItem(int iPosition)
        throws IllegalArgumentException
    {
        // Make sure the position is valid
        if ((iPosition < B1) || (iPosition > B3)) {
            throw new IllegalArgumentException("invalid position index") ;
        }


        // remove any item that may have already been added
        if (m_labels[iPosition] != null) {
            remove(m_labels[iPosition]) ;
            m_labels[iPosition] = null ;
            m_actionCommands[iPosition] = null ;
            m_hints[iPosition] = null ;

            //remove all listeners for this button
            m_vListeners[iPosition].removeAllElements();


            Rectangle rectPosition = getComponentLocationAt(iPosition) ;
            repaint(rectPosition.x, rectPosition.y, rectPosition.width, rectPosition.height) ;
        }
    }





    /**
     * Add an action listener to the bottom button bar.  Listeners will
     * receive notification whenever a button is pressed.  If more than one
     * listener is added, only the last one is active.
     *
     * @param listener Listener interested in receiving button pressed
     *        notifications.
     */
    public void addActionListener(PActionListener listener)
    {
        /* for now, add the listener to all buttons */
        m_vGlobalListener.addElement(listener) ;
    }


    /**
     * Remove an action listener from the bottom button bar.
     *
     * @param listener The {@link PActionListener} that should be removed
     *        from the listener list.
       */
    public void removeActionListener(PActionListener listener)
    {
        m_vGlobalListener.removeElement(listener) ;

    }

    /**
     * @deprecated Hide from users
     */
    public void doLayout()
    {
        layoutComponents() ;
        super.doLayout() ;
    }

    /**
     * Override removeAll because hints don't get removed if you just call removeAll
     * on the base class.
     */
    public void removeAll()
    {
        super.removeAll();
        clearItem(B1);
        clearItem(B2);
        clearItem(B3);
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////



    /**
     *
     */
    private void fireActionEvent(int iButtonId, String strActionCommand)
    {
        PActionEvent    event = new PActionEvent(this, strActionCommand) ;
        PActionListener listener ;

        /* go through gobal listeners */
        for (   Enumeration e = m_vGlobalListener.elements(); e.hasMoreElements(); ) {

            listener = (PActionListener) e.nextElement() ;

            if (listener != null) {
                try {
                    listener.actionEvent(event) ;
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }

            // Check to see if our listener gobbled up the event
            if (event.isConsumed())
                break ;
        }

        if (event.isConsumed())
            return;

        // Tell everyone in the category about the event
        for (   Enumeration e = m_vListeners[iButtonId].elements(); e.hasMoreElements(); ) {

            listener = (PActionListener) e.nextElement() ;

            if (listener != null) {
                try {
                    listener.actionEvent(event) ;
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }

            // Check to see if our listener gobbled up the event
            if (event.isConsumed())
                break ;
        }
    }


/////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     *
     */
    private void layoutComponents()
    {
        Rectangle rectArea = getBounds() ;
        int       iWidth = rectArea.width / 3 ;
        int       iHeight = rectArea.height ;


        for (int i=0; i<3; i++) {
            if (m_labels[i] != null) {
                m_labels[i].setBounds(getComponentLocationAt(i)) ;
            }
        }
    }


    /**
     * What is the rectangle/location of the specified position
     */
    private Rectangle getComponentLocationAt(int iPosition)
    {
        Rectangle rectArea = getBounds() ;
        int       iWidth = rectArea.width / 3 ;

        rectArea.x = iWidth*iPosition ;
        rectArea.y = 0 ;
        rectArea.width = iWidth ;

        return rectArea ;
    }


    protected void dump()
    {
        for (int i=0; i<MAX_BUTTONS; i++) {
            System.out.println(Integer.toString(i) + ": " + m_labels[i] + " Enable=" + m_bEnabled[i]) ;

        }
    }

//////////////////////////////////////////////////////////////////////////////
// Inner classes
////
    private class icButtonListener implements PButtonListener
    {
        public void buttonDown(PButtonEvent event)
        {
            // avoid multiple tooltip requests
            if (m_iButtonDown != -1) {
                return ;
            }

            switch (event.getButtonID()) {
                case PButtonEvent.BID_B1:
                    // Start up a timer for a tooltip
                    if (m_bEnabled[0]) {
                        m_iButtonDown = event.getButtonID() ;
                        Timer.getInstance().addTimer(Settings.getInt("TOOLTIP_TIMEOUT_LENGTH", 1000), m_tooltipHandler, new Integer(0)) ;
                    }
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_B2:
                    // Start up a timer for a tooltip
                    if (m_bEnabled[1]) {
                        m_iButtonDown = event.getButtonID() ;
                        Timer.getInstance().addTimer(Settings.getInt("TOOLTIP_TIMEOUT_LENGTH", 1000), m_tooltipHandler, new Integer(1)) ;
                    }
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_B3:
                    // Start up a timer for a tooltip
                    if (m_bEnabled[2]) {
                        m_iButtonDown = event.getButtonID() ;
                        Timer.getInstance().addTimer(Settings.getInt("TOOLTIP_TIMEOUT_LENGTH", 1000), m_tooltipHandler, new Integer(2)) ;
                    }
                    event.consume() ;
                    break ;
            }

        }


        public void buttonUp(PButtonEvent event)
        {
            // Release any tooltip timers
            if (m_iButtonDown != -1)
            {
                m_iButtonDown = -1 ;
                Timer.getInstance().removeTimers(m_tooltipHandler) ;
            }

            switch (event.getButtonID()) {
                case PButtonEvent.BID_B1:
                    if ((m_actionCommands[0] != null) && m_bEnabled[0]) {
                        fireActionEvent(0,m_actionCommands[0]) ;
                    }
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_B2:
                    if ((m_actionCommands[1] != null) && m_bEnabled[1]) {
                        fireActionEvent(1,m_actionCommands[1]) ;
                    }
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_B3:
                    if ((m_actionCommands[2] != null) && m_bEnabled[2]) {
                        fireActionEvent(2,m_actionCommands[2]) ;
                    }
                    event.consume() ;
                    break ;
            }
        }


        public void buttonRepeat(PButtonEvent event)
        {
        }

    }


    private class icTooltipHandler implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            int iButton = ((Integer) event.getObjectParam()).intValue() ;

            if ((m_labels[iButton] != null) && (m_iButtonDown != -1))
            {
                if ((m_hints[iButton] != null) && (m_hints[iButton].length() > 0)) {
                    Shell.getInstance().displayHint(m_hints[iButton]) ;
                } else {
                    Shell.getInstance().displayHint(AppResourceManager.getInstance().getString("lblNoHintAvailable")) ;
                }
            }
            m_iButtonDown = -1 ;
        }
    }
}
