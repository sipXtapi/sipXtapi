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

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.service.* ;

import java.awt.* ;
import java.util.Vector ;
import java.util.Enumeration ;

import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.sys.* ;


/**
 * The PCommandBar is a vertical button bar located on the left side of the
 * form. It has enough room to display four labels or icons at a time
 * representing possible user actions.
 * <p>
 * Notification of button presses is made by firing off PActionEvents.
 * Developers can choose to subscribe to the entire button bar or to a
 * specified particular action.
 *
 * @see PActionEvent
 * @author Robert J. Andreasen, Jr.
 */
public class PCommandBar extends PContainer
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** size of our button bar area */
    private static final Dimension DIM_BUTTON_SIZE = new Dimension(27,108) ;

    /** max number of buttons that can be handled by our button bar is 4 pages or 12 buttons */
    private static final int MAX_BUTTONS = 12 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /**
     * in cases where we have more than N buttons, we have a specific button
     * that cycles us through the others.  m_iButtonTop is the button index
     * for the topmost button being displayed after cycling to whatever page
     */
    protected int m_iButtonTop = 0 ;

    /** are we in cycle mode */
    protected boolean m_bCycleMode = false ;

    /** Vector of buttons descriptors */
    protected Vector m_vButtons = new Vector() ;

    /** Vector of general action listeners */
    protected Vector m_vListeners = new Vector() ;

    /** internal list of button descriptors */
    private icButtonDescriptor m_buttonDescriptor[] = new icButtonDescriptor[MAX_BUTTONS] ;

    /** current button labels */
    protected PLabel m_vListItems[] = new PLabel[4] ;
    /** current button action commands */
    protected String m_vActionCommands[] = new String[4] ;
    /** current button tooltips */
    protected String m_strTooltips[] = new String[4] ;

    /** Standard background */
    private Image  m_imgButtonBackground ;

    protected int               m_iButtonDown = -1 ;
    protected icTooltipHandler  m_tooltipHandler = new icTooltipHandler() ;

    protected icButtonDescriptor m_cycleDescriptor = null;


    /**
     * Default, no argument constructor.
     */
    public PCommandBar()
    {
        // Load the background image for our buttons
        m_imgButtonBackground = getImage("imgButtonBar") ;
        setOpaque(false) ;

        layoutComponents() ;
        addButtonListener(new icButtonListener()) ;
    }


    /**
     * Add a button to the PCommandBar.
     *
     * @param label Text or graphic representation of the button.
     * @param strActionCommand The 'action' or command name used for identification
     *        by the listeners.
     * @param strHint The information that displays about this action when a user
     *        holds down the button. To include a title as well as text, use the
     *        format <hint title>|<hint text>.
     */
    public void addButton(PLabel label, String strActionCommand, String strHint)
    {
        int                 iPosition = m_vButtons.size() ;
        icButtonDescriptor  desc ;

        // Add to button list
        desc = new icButtonDescriptor(  label,
                                        strActionCommand,
                                        strHint,
                                        null) ;
        m_vButtons.addElement(desc) ;

        // Add to initial display if before position 4
        if (iPosition < 4) {
            setButton(iPosition, desc) ;
        } else if (iPosition == 4) {
            initCycleMode() ;
        }
    }



    /**
     * Add a button to the PCommandBar.
     *
     * @param item The {@link PActionItem} containing label, action command, hint,
     *        and an optional action listener.
     *
     * @exception IllegalArgumentException Thrown if too many buttons are
     *            added to the control.
     */
    public void addButton(PActionItem item)
    {
        int                 iPosition = m_vButtons.size() ;
        icButtonDescriptor  desc ;

        if (iPosition >= MAX_BUTTONS) {
            throw new IllegalArgumentException("Button Bar is configured to handle a max of " + MAX_BUTTONS + " buttons.") ;
        } else {

            // Add to button list
            desc = new icButtonDescriptor(  item.getLabel(),
                                            item.getActionCommand(),
                                            item.getHint(),
                                            item.getActionListener() ) ;


            m_vButtons.addElement(desc) ;

            // Add to initial display if before position 4
            if (iPosition < 4) {
                setButton(iPosition, desc) ;
            } else if (iPosition == 4) {
                initCycleMode() ;
            }
            enableByAction(item.getActionCommand(),item.isEnabled()) ;
        }
    }


    /**
     * Remove all buttons from the control.
     */
    public void removeAllButtons()
    {
        int i ;

        if ((m_vButtons != null) && (m_vButtons.size() > 0)) {

            // reset various state variables
            m_iButtonTop = 0 ;
            m_bCycleMode = false ;
            m_vButtons.removeAllElements() ;
            m_cycleDescriptor = null ;

            // Clear button descriptors
            if (m_buttonDescriptor != null) {
                for (i=0; i<MAX_BUTTONS; i++) {
                    if (m_buttonDescriptor[i] != null) {
                        if (m_buttonDescriptor[i].isEnabled()) {
                            remove(m_buttonDescriptor[i].m_lblButton) ;
                        }
                        m_buttonDescriptor[i] = null ;
                    }
                }


            }

            // clear current view of the world
            for (i=0; i<4; i++) {
                if (m_vListItems != null)
                    m_vListItems[i] = null ;
                if (m_vActionCommands != null)
                    m_vActionCommands[i] = null ;
                if (m_strTooltips != null)
                    m_strTooltips[i] = null ;
            }
            repaint() ;
        }
    }


    /**
     * Enable or disable a button by its action string.  Disabled buttons
     * will become invisible; however, developers cannot reuse that button area
     * for another button.
     */
    public synchronized void enableByAction(String strActionCommand, boolean bEnable)
    {
        if (bEnable) {
            enableByAction(strActionCommand) ;
        } else {
            disableByAction(strActionCommand) ;
        }
    }


    /**
     * Add an action listener to the command bar.  Listeners will receive
     * notification whenever a button is pressed.  Adding a listener twice
     * results in a silent error.
     *
     * @param listener Listener interested in receiving button press
     *         notifications.
     */
    public void addActionListener(PActionListener listener)
    {
        if (!m_vListeners.contains(listener))
            m_vListeners.addElement(listener) ;
    }


    /**
     * Remove an action listener from the command bar.
     *
     * @param listener The {@link PActionListener} that should be removed from
     *        the listener list.
     */
    public void removeActionListener(PActionListener listener)
    {
        m_vListeners.removeElement(listener) ;
    }


    /**
     * Update the button label for all items with the specified action
     */
    public void updateButtonLabelByAction(String strActionCommand, String strLabel)
    {
        Enumeration enum = m_vButtons.elements() ;

        while (enum.hasMoreElements()) {
            icButtonDescriptor desc = (icButtonDescriptor) enum.nextElement() ;

            if (desc.m_strActionCommand.equalsIgnoreCase(strActionCommand)) {
                int iPosition = desc.getPosition() ;
                if ((iPosition != -1) && desc.isEnabled()) {
                    remove(desc.m_lblButton) ;
                    desc.m_lblButton = new PLabel(strLabel) ;
                    setButton(iPosition, desc) ;
                } else {
                    desc.m_lblButton = new PLabel(strLabel) ;
                }
            }
        }
    }

    /**
     * Update the button label for all items with the specified action
     */
    public void updateButtonLabelByAction(String strActionCommand, PLabel label)
    {
        Enumeration enum = m_vButtons.elements() ;

        while (enum.hasMoreElements()) {
            icButtonDescriptor desc = (icButtonDescriptor) enum.nextElement() ;

            if (desc.m_strActionCommand.equalsIgnoreCase(strActionCommand)) {
                int iPosition = desc.getPosition() ;
                if ((iPosition != -1) && desc.isEnabled()) {
                    remove(desc.m_lblButton) ;
                    desc.m_lblButton = label;
                    setButton(iPosition, desc) ;
                } else {
                    desc.m_lblButton = label;
                }
            }
        }
    }

    /**
     * Update the button hint where the action command matches
     * @deprecated do not expose
     */
    public void updateButtonHintByAction(String strActionCommand, String strHint)
    {
        Enumeration enum = m_vButtons.elements() ;

        while (enum.hasMoreElements()) {
            icButtonDescriptor desc = (icButtonDescriptor) enum.nextElement() ;

            if (desc.m_strActionCommand.equalsIgnoreCase(strActionCommand)) {
                int iPosition = desc.getPosition() ;
                if (iPosition != -1)
                {
                    desc.m_strTooltip = strHint;
                }
            }
        }
    }

    /**
     * Paint this component. This method is called when the contents of the
     * component should be painted in response to either the component first being
     * shown, or damage needing repair. The clip rectangle in the Graphics
     * parameter will be set to the area which needs to be painted.
     *
     * @deprecated Hide
     */
    public void paint(Graphics g)
    {
        // Draw the button background
        g.drawImage(m_imgButtonBackground, 0, 0, this) ;

        // Allow other components to paint...
        super.paint(g) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     *
     */
    private void layoutComponents()
    {
        setLayout(null) ;
    }


    /**
     * fire off an action event given the passed descriptor
     */
    private void fireActionEvent(icButtonDescriptor  desc)
    {
        if (desc.m_strActionCommand != null) {
            PActionEvent event = new PActionEvent(this, desc.m_strActionCommand) ;

            // fire off any a specific listener
            if (desc.m_listener != null) {
                try {
                    desc.m_listener.actionEvent(event) ;
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }

            // fire off to the general population
            Enumeration enum = m_vListeners.elements() ;
            while (enum.hasMoreElements()) {
                PActionListener listener = (PActionListener) enum.nextElement() ;
                if (listener != null) {
                    try {
                        listener.actionEvent(event) ;
                    } catch (Throwable t) {
                        Shell.getInstance().showUnhandledException(t, true) ;
                    }
                }
            }
        }
    }


    /**
     * enable a previous disabled button.
     */
    private void enableByAction(String strActionCommand)
    {
        Enumeration enum = m_vButtons.elements() ;

        while (enum.hasMoreElements()) {
            icButtonDescriptor desc = (icButtonDescriptor) enum.nextElement() ;

            if (desc.m_strActionCommand.equalsIgnoreCase(strActionCommand)) {
                int iPosition = desc.getPosition() ;

                if (iPosition != -1) {
                    if (!desc.isEnabled()) {
                        add(desc.m_lblButton) ;
                        setButton(iPosition, desc) ;
                    }
                }
                desc.setEnabled(true) ;
            }
        }
    }


    /**
     * We are now in "cycle" mode where the user can cycle between various
     * pages of icons/commands
     */
    private void initCycleMode()
    {
        if (m_cycleDescriptor == null) {
            m_cycleDescriptor = new icButtonDescriptor( new PLabel(getImage("imgCycleIcon")),
                                                        null,
                                                        "Cycle to other possible actions",
                                                        null) ;
        }

        m_bCycleMode = true ;
        setButton(3, m_cycleDescriptor) ;
    }


    /**
     * Cycle the list of command bar buttons
     */
    private void doCycle()
    {
        int iNumButtons = m_vButtons.size() ;

        if ((m_iButtonTop + 3) > iNumButtons)
            m_iButtonTop = 0 ;
        else
            m_iButtonTop += 3 ;

        for (int i=0; i<3; i++) {
            if (m_iButtonTop+i < iNumButtons)
                setButton(i, (icButtonDescriptor)  m_vButtons.elementAt(m_iButtonTop+i)) ;
            else
                clearButton(i) ;
        }
    }


    /**
     * disable a button.  This will remove the button until enabled, however,
     * the button will not/cannot be reused by another button.
     */
    private void disableByAction(String strActionCommand)
    {
        Enumeration enum = m_vButtons.elements() ;

        while (enum.hasMoreElements()) {
            icButtonDescriptor desc = (icButtonDescriptor) enum.nextElement() ;

            if (desc.m_strActionCommand.equalsIgnoreCase(strActionCommand)) {
                if (desc.isEnabled()) {
                    int iPosition = desc.getPosition() ;
                    if (iPosition != -1) {
                        remove(desc.m_lblButton) ;
                        Rectangle rectBounds = getBoundsForButton(iPosition) ;
                        repaint(rectBounds.x, rectBounds.y, rectBounds.width, rectBounds.height) ;
                    }

                    desc.setEnabled(false) ;
                }
            }
        }
    }


    /**
     * clear a button at position iPosition
     */
    private synchronized void clearButton(int iPosition)
    {
        // Remove existing...
        if (m_buttonDescriptor[iPosition] != null) {
            // We are only added / displayed if enabled
            if (m_buttonDescriptor[iPosition].isEnabled()) {
                remove(m_buttonDescriptor[iPosition].m_lblButton) ;
            }
            m_buttonDescriptor[iPosition].setPosition(-1) ;
            m_buttonDescriptor[iPosition] = null ;

            Rectangle rectBounds = getBoundsForButton(iPosition) ;
            repaint(rectBounds.x, rectBounds.y, rectBounds.width, rectBounds.height) ;
        }
    }


    /**
     * update a button given the passed position and descriptor.
     */
    private synchronized void setButton(int iPosition, icButtonDescriptor desc)
    {
        clearButton(iPosition) ;

        // Add new
        if (desc != null) {
            desc.m_lblButton.setBounds(getBoundsForButton(iPosition)) ;
            // Only add if enabled
            if (desc.isEnabled()) {
                add(desc.m_lblButton) ;
            }

            m_buttonDescriptor[iPosition] = desc ;
            m_buttonDescriptor[iPosition].setPosition(iPosition) ;

            desc.m_lblButton.repaint() ;
        }
    }


    /**
     * Get the bounding rectangle for the button at iPosition
     */
    private Rectangle getBoundsForButton(int iPosition)
    {
        return new Rectangle(0, (iPosition*27), 27, 28) ;
    }



//////////////////////////////////////////////////////////////////////////////
// Innerclasses
////
    private class icButtonListener implements PButtonListener
    {
        /**
         * The specified button has been press downwards
         */
        public void buttonDown(PButtonEvent event)
        {
            int iTooltipTimeout = Settings.getInt("TOOLTIP_TIMEOUT_LENGTH", 1000) ;

            // avoid multiple tooltip requests
            if (m_iButtonDown != -1) {
                return ;
            }

            switch (event.getButtonID()) {
                case PButtonEvent.BID_L1:
                    // Start up a timer for a tooltip
                    if ((m_buttonDescriptor[0] != null) && m_buttonDescriptor[0].isEnabled()) {
                        m_iButtonDown = event.getButtonID() ;
                        Timer.getInstance().addTimer(iTooltipTimeout, m_tooltipHandler, new Integer(0)) ;
                    }
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_L2:
                    // Start up a timer for a tooltip
                    if ((m_buttonDescriptor[1] != null) && m_buttonDescriptor[1].isEnabled()) {
                        m_iButtonDown = event.getButtonID() ;
                        Timer.getInstance().addTimer(iTooltipTimeout, m_tooltipHandler, new Integer(1)) ;
                    }
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_L3:
                    // Start up a timer for a tooltip
                    if ((m_buttonDescriptor[2] != null) && m_buttonDescriptor[2].isEnabled()) {
                        m_iButtonDown = event.getButtonID() ;
                        Timer.getInstance().addTimer(iTooltipTimeout, m_tooltipHandler, new Integer(2)) ;
                    }
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_L4:
                    // Start up a timer for a tooltip
                    if ((m_buttonDescriptor[3] != null) && m_buttonDescriptor[3].isEnabled()) {
                        m_iButtonDown = event.getButtonID() ;
                        Timer.getInstance().addTimer(iTooltipTimeout, m_tooltipHandler, new Integer(3)) ;
                    }
                    event.consume() ;
                    break ;
            }
        }


        /**
         * The specified button has been released
         */
        public void buttonUp(PButtonEvent event)
        {
            // Release any tooltip timers
            if (m_iButtonDown != -1) {
                Timer.getInstance().removeTimers(m_tooltipHandler) ;
                m_iButtonDown = -1 ;
            }

            switch (event.getButtonID()) {
                case PButtonEvent.BID_L1:
                    if ((m_buttonDescriptor[0] != null) && m_buttonDescriptor[0].isEnabled()) {
                        fireActionEvent(m_buttonDescriptor[0]) ;
                    }
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_L2:
                    if ((m_buttonDescriptor[1] != null) && m_buttonDescriptor[1].isEnabled()) {
                        fireActionEvent(m_buttonDescriptor[1]) ;
                    }
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_L3:
                    if ((m_buttonDescriptor[2] != null) && m_buttonDescriptor[2].isEnabled()) {
                        fireActionEvent(m_buttonDescriptor[2]) ;
                    }
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_L4:
                    if (m_bCycleMode) {
                        doCycle() ;
                    } else {
                        if ((m_buttonDescriptor[3] != null) && m_buttonDescriptor[3].isEnabled()) {
                            fireActionEvent(m_buttonDescriptor[3]) ;
                        }
                    }
                    event.consume() ;
                    break ;
            }
        }


        /**
         * The specified button is being held down
         */
        public void buttonRepeat(PButtonEvent event)
        {

        }
    }



    /**
     * this action listener is called after a tooltip timeout
     */
    private class icTooltipHandler implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            int iButton = ((Integer) event.getObjectParam()).intValue() ;

            // Display a tooltip
            if (m_buttonDescriptor[iButton] != null) {
                if (m_buttonDescriptor[iButton].m_strTooltip != null) {
                    Shell.getInstance().displayHint(m_buttonDescriptor[iButton].m_strTooltip) ;
                } else {
                    Shell.getInstance().displayHint(AppResourceManager.getInstance().getString("lblNoHintAvailable")) ;
                }
            }
            m_iButtonDown = -1 ;
        }
    }


    /**
     * A button descriptor stores the various pieces of information about a
     * button.  This includes the label, action command, specific action
     * listener, tooltip text, and current button pos, enabled/disabled
     * state.
     */
    protected class icButtonDescriptor
    {
        /** button attribute: label */
        public PLabel           m_lblButton ;
        /** button attribute: action command */
        public String           m_strActionCommand ;
        /** button attribute: tooltip string */
        public String           m_strTooltip ;
        /** button attribute: listener */
        public PActionListener  m_listener ;

        /** is this button enabled or not? */
        public boolean          m_bEnabled ;
        /** what button position is this currently in?  or -1 if not displayed */
        public int              m_iPosition ;


        /**
         * only constructor mandidates button attributes.  Doesn't make sense
         * for a default constructor.  Buttons default to be enabled unless
         * specifically disabled.
         */
        public icButtonDescriptor(  PLabel          lblButton,
                                    String          strActionCommand,
                                    String          strTooltip,
                                    PActionListener listener)
        {
            m_lblButton = lblButton ;
            m_strActionCommand = strActionCommand ;
            m_strTooltip = strTooltip ;
            m_listener = listener ;

            m_bEnabled = true ;
            m_iPosition = -1 ;
        }


        /**
         * enable or disable the descriptor button state
         */
        public void setEnabled(boolean bEnable)
        {
            m_bEnabled = bEnable ;
        }

        /**
         * what is the state of the current button?
         */
        public boolean isEnabled()
        {
            return m_bEnabled ;
        }


        /**
         * set the current position of this button.  Set to -1 if button
         * is not being displayed on the screen
         */
        public void setPosition(int iPosition)
        {
            m_iPosition = iPosition ;
        }


        /**
         * what is the current position of this button?
         *
         * @return current position or -1 if not currently being displayed
         */
        public int getPosition()
        {
            return m_iPosition ;
        }


        /**
         * Debugging methods that dumps the state of this descriptor
         */
        protected void dump()
        {
            System.out.println("") ;
            System.out.println("icButtonDescriptor:") ;
            System.out.println("    label: " + m_lblButton) ;
            System.out.println("   action: " + m_strActionCommand) ;
            System.out.println("  tooltip: " + m_strTooltip) ;
            System.out.println(" listener: " + m_listener) ;
            System.out.println(" position: " + m_iPosition) ;
            System.out.println("  enabled: " + m_bEnabled) ;
        }

    }
}
