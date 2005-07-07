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
import java.util.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.sys.* ;

/**
 * A Scrollable component container is a special container that can hold
 * many components while enforcing a set layout. The scrollable component
 * handles scrolling internally and also understands the concept of focus
 * and directs PFocusTraversable components.
 * <p>
 * In addition to the base functionality, the scrollable component container
 * has convenience methods for adding labels above text fields and check
 * boxes. Special addComponent(PCheckbox, PLabel) and
 * addComponent(PTextField, PLabel) methods have been implemented.  It would
 * be useful to add some sort of alignment field later.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PScrollableComponentContainer extends PContainer
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private int         m_iTopIndex ;           // index of element at top of view port
    private int         m_iDisplayableItems ;   // how many items we can display at once
    private Vector      m_vMembers ;            // actual list of components/members
    private Component   m_surfacedComponents[] ;// array of surfaced (viewing) components
    private PScrollbar  m_scrollbar ;           // our scrollbar
    private int         m_iStartingButtonId ;   // Starting button ID of this component
    private boolean     m_bSetInitialFocus ;    // Have we given a component focus?
    private icFocusMonitor m_focusMonitor ;     // Monitors focus changes

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Create a scrollable component container that can display the specified
     * number of components (iHeight) before requiring a scroll bar.
     *
     * @param iHeight The maximum number of items that can be displayed
     *        simultaneously.
     * @param iStartingButtonId The starting button id of the container.  If
     *        the container is position at topmost position, it will use
     *        BID_R1, however, if the top of this container is place in the
     *        middle of the screen, the position should be set to BID_R3.
     */
    public PScrollableComponentContainer(int iHeight, int iStartingButtonId)
    {
        // Make sure the iHeight make sense
        if ((iHeight < 1) || (iHeight > 4)) {
            throw new IllegalArgumentException("Cannot only display 1-4 items") ;
        }

        m_iStartingButtonId = iStartingButtonId ;
        m_iDisplayableItems = iHeight ;
        m_iTopIndex = 0 ;
        m_vMembers = new Vector() ;
        m_surfacedComponents = new Component[m_iDisplayableItems] ;
        m_scrollbar = new PScrollbar() ;

        addButtonListener(new icButtonListener()) ;
        m_focusMonitor = new icFocusMonitor() ;
        m_bSetInitialFocus = false ;
    }


    /**
     * Add the specified component to the end (last position) of this
     * container.  If the component argument is null, nothing is completed.
     *
     * @param component component that will be added to this container
     */
    public void addComponent(Component component)
    {
        if (component != null) {
            m_vMembers.addElement(component) ;

            // Focus is turned 'off' whenever a component is added
            if (component instanceof PFocusTraversable) {
                ((PFocusTraversable) component).addFocusListener(m_focusMonitor) ;
                ((PFocusTraversable) component).setFocus(false) ;
            }
        }

        fixupScrollPosition() ;
        surfaceComponents() ;
    }


    /**
     * Insert the specified component into this container at the specified
     * index position. If the index position is invalid (less than zero or
     * greater than the current number of elements) an
     * ArrayIndexOutOfBoundsException exception is thrown. If the component
     * argument is null, nothing is completed.
     *
     * @param component The component that will be added to this container.
     * @param index The index position at which the component will be inserted.
     *
     * @exception ArrayIndexOutOfBoundsException Thrown when the specified
     *            index position is either less than zero or greater than the
     *            total number of components in this container.
     */
    public void insertComponentAt(Component component, int index)
        throws ArrayIndexOutOfBoundsException
    {
        if (component != null) {
            m_vMembers.insertElementAt(component, index) ;

            // Focus is turned 'off' whenever a component is added
            if (component instanceof PFocusTraversable) {
                ((PFocusTraversable) component).addFocusListener(m_focusMonitor) ;
                ((PFocusTraversable) component).setFocus(false) ;
            }
        }

        fixupScrollPosition() ;
        surfaceComponents() ;
    }


    /**
     * Insert the specified 'new element' immediately after the specified
     * 'existing object'.
     *
     * @param compNewElement The new element that will be added to the list.
     * @param compExistingElement The element the new element will be inserted
     *        after.
     *
     * @exception IllegalArgumentException if the existing element is not a
     *            member of this list.
     *
     */
    public synchronized void insertComponentAfter(Component compNewElement, Component compExistingElement)
        throws IllegalArgumentException
    {
        int iPosition = m_vMembers.indexOf(compExistingElement) ;
        if (iPosition == -1) {
            throw new IllegalArgumentException("existing component is not a member of this list") ;
        } else {
            if (compNewElement instanceof PFocusTraversable) {
                ((PFocusTraversable) compNewElement).addFocusListener(m_focusMonitor) ;
                ((PFocusTraversable) compNewElement).setFocus(false) ;
            }

            m_vMembers.insertElementAt(compNewElement, iPosition+1) ;
            fixupScrollPosition() ;
            surfaceComponents() ;
        }
    }


    /**
     * Insert the specified 'new element' immediately before the specified
     * 'existing object'.
     *
     * @param compNewElement The new element that will be added to the list.
     * @param compExistingElement The element the new element will be inserted
     *        after.
     *
     * @exception IllegalArgumentException if the existing element is not a
     *            member of this list.
     *
     */
    public synchronized void insertComponentBefore(Component compNewElement, Component compExistingElement)
        throws IllegalArgumentException
    {
        int iPosition = m_vMembers.indexOf(compExistingElement) ;
        if (iPosition == -1) {
            throw new IllegalArgumentException("existing component is not a member of this list") ;
        } else {
            if (compNewElement instanceof PFocusTraversable) {
                ((PFocusTraversable) compNewElement).addFocusListener(m_focusMonitor) ;
                ((PFocusTraversable) compNewElement).setFocus(false) ;
            }

            m_vMembers.insertElementAt(compNewElement, iPosition) ;
            fixupScrollPosition() ;
            surfaceComponents() ;
        }
    }


    /**
     * Remove the specified component from this container. If the component
     * is not a member of this container or the component is null, no action
     * is performed.
     *
     * @param component component that will be removed from this container
     *
     * @return boolean true if the removal was successful otherwise false
     */
    public boolean removeComponent(Component component)
    {
        boolean bSuccess = false ;

        if (component != null) {
            bSuccess = m_vMembers.removeElement(component) ;

            // Make sure focus is disabled and stop monitoring this component
            // for focus changes.
            if (component instanceof PFocusTraversable) {
                ((PFocusTraversable) component).removeFocusListener(m_focusMonitor) ;
                ((PFocusTraversable) component).setFocus(false) ;
            }
        }

        fixupScrollPosition() ;
        surfaceComponents() ;

        return bSuccess ;
    }


    /**
     * Remove the component at the specified index from this container. A
     * ArrayIndexOutOfBoundsException is thrown if the index is invalid (less
     * then zero or greater than or equal to the total number of components
     * in the container).
     *
     * @param index index position of component that will be removed from this
     *        container
     */
    public void removeComponentAt(int index)
        throws ArrayIndexOutOfBoundsException
    {
        Component component = (Component) m_vMembers.elementAt(index) ;
        removeComponent(component) ;
    }


    /**
     * Searches for the occurence of the specified componet within this
     * container. Equality is checked by the equals methods.
     *
     * @param component Component whose index value will be returned
     *
     * @return index Value of the specified component or -1 if the specified
     *         component is not found
     */
    public int indexOf(Component component)
    {
        return m_vMembers.indexOf(component) ;
    }


    /**
     * Finds the index of the first item in the view port.
     * @return The index of the first item in view. If there are no items,
     * it returns 0.
     */
    protected int getFirstVisibleIndex()
    {
        return  m_iTopIndex  ;
    }

    /**
     * Returns the number of items that can be displayed in this container.
     * This value is set by the parameter "iHeight"
     * when the container is created.
     *
     */
    protected int getVisibleRowCount(){
        return  m_iDisplayableItems;
    }


    /**
     * Returns the component at the specified index position.  An
     * ArrayIndexOutOfBoundsException is thrown if the specified index
     * position is less than zero or greater than or equal to the total
     * number of components in this container.
     *
     * @param index index position of the component that will be returned
     *
     * @throw ArrayIndexOutOfBoundsException thrown when the specified index
     *        position is less than zero or greater than or equal to the total
     *        number of components in this container
     */
    public Component componentAt(int index)
        throws ArrayIndexOutOfBoundsException
    {
        return (Component) m_vMembers.elementAt(index) ;
    }


    /**
     * Replaces the component at the specified index position
     *
     */
    public void setComponentAt(Component component, int index)
    {
        m_vMembers.setElementAt(component, index) ;

        // TODO: Only surface if within view
        surfaceComponents() ;
    }


    public int getLength()
    {
        return m_vMembers.size() ;
    }


    /**
     * Called by the framework when it is time to phyiscally layout our
     * components.
     *
     * @deprecated do not expose
     */
    public void doLayout()
    {
        int iHeight = getSize().height / m_iDisplayableItems ;
        int iWidth = getSize().width ;

        boolean bNeedScrollbar = false ;

        if (m_vMembers.size() > m_iDisplayableItems) {
            bNeedScrollbar = true ;
        }

        // Add/Remove Scroll bar as required
        if (bNeedScrollbar) {
            if (!GUIUtils.componentInContainer(m_scrollbar, this)) {
                add(m_scrollbar) ;
            }
        } else {
            remove(m_scrollbar) ;
        }

        // Layout components
        if (bNeedScrollbar) {
            // With scroll bar
            m_scrollbar.setBounds(0, 0, SystemDefaults.getMetric(SystemDefaults.METRICID_SCROLLBAR_WIDTH),
SystemDefaults.getMetric(SystemDefaults.METRICID_SCROLLBAR_HEIGHT)-5) ;
            for (int i=0;i<m_iDisplayableItems; i++) {
                if (m_surfacedComponents[i] != null) {
                    m_surfacedComponents[i].setBounds(SystemDefaults.getMetric(SystemDefaults.METRICID_SCROLLBAR_WIDTH), i*iHeight, iWidth-SystemDefaults.getMetric(SystemDefaults.METRICID_SCROLLBAR_WIDTH), iHeight) ;
                    m_surfacedComponents[i].doLayout() ;
                }
            }
        } else {
            // Without scroll bar
            for (int i=0;i<m_iDisplayableItems; i++) {
                if (m_surfacedComponents[i] != null) {
                    m_surfacedComponents[i].setBounds(0, i*iHeight, iWidth, iHeight) ;
                    m_surfacedComponents[i].doLayout() ;
                }
            }
        }
        super.doLayout() ;
    }

    public Component getComponentAt(int iPosition) {

          if (iPosition < m_surfacedComponents.length) {
            return m_surfacedComponents[iPosition] ;
          }
          else {
            return null ;
          }


    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
/////
    /**
     * Assigns and lays out the components for physical display.  This includes
     * updating the layout, assigning button ids, and tweaking focus.  This
     * method is blind and will relayout check boxes even if the no changes
     * have been made.
     */
    protected void surfaceComponents()
    {
        // Remove/sink any surfaced components.
        for (int i=0; i<m_iDisplayableItems; i++) {
            if (m_surfacedComponents[i] != null) {

                // We need to do something special for check boxes
                if (m_surfacedComponents[i] instanceof PCheckbox)
                    ((PCheckbox) m_surfacedComponents[i]).setAssociatedButton(-1) ;
                // We need to do something special for choice controls
                if (m_surfacedComponents[i] instanceof PChoice)
                    ((PChoice) m_surfacedComponents[i]).setAssociatedButton(-1) ;


                remove(m_surfacedComponents[i]) ;
                m_surfacedComponents[i] = null ;
            }
        }

        // Populate surfaced check box list
        int iNumItems = m_vMembers.size() ;
        PFocusTraversable focusHolder = m_focusMonitor.getFocusHolder() ;
        boolean m_bFocusHolderFound = false ;
        for (int i=0; i<m_iDisplayableItems; i++) {
            if ((i+m_iTopIndex) < iNumItems) {
                m_surfacedComponents[i] = (Component) m_vMembers.elementAt(i+m_iTopIndex) ;

                // We need to do something special for check boxes
                if (m_surfacedComponents[i] instanceof PCheckbox)
                    ((PCheckbox) m_surfacedComponents[i]).setAssociatedButton(m_iStartingButtonId + i) ;
                // We need to do something special for choice controls
                if (m_surfacedComponents[i] instanceof PChoice)
                    ((PChoice) m_surfacedComponents[i]).setAssociatedButton(m_iStartingButtonId + i) ;

                add(m_surfacedComponents[i]) ;

                // Note if the focus holder is on screen
                if (focusHolder == m_surfacedComponents[i])
                    m_bFocusHolderFound = true ;

                // Set initial focus if it has not yet set.
                if (m_bSetInitialFocus == false) {
                    if (m_surfacedComponents[i] instanceof PFocusTraversable) {
                        ((PFocusTraversable) m_surfacedComponents[i]).setFocus(true) ;
                        m_bSetInitialFocus = true ;
                    }
                }
            }
        }

        // If the focus holder has moved off screen, remove focus.
        if ((!m_bFocusHolderFound) && (focusHolder != null))
            focusHolder.setFocus(false) ;

        if (m_vMembers.size() > 4) {
            m_scrollbar.setNumPosition(m_vMembers.size()-4) ;
            m_scrollbar.setPosition(m_iTopIndex) ;
        }

        if (isVisible()) {
            doLayout() ;
            repaint() ;
        }
    }


    /**
     * Scrolls our viewport upwards by 1 row.
     */
    protected void onScrollUp()
    {
        if (m_iTopIndex > 0) {
            m_iTopIndex-- ;
            surfaceComponents() ;
        }
    }


    /**
     * Scrolls our viewport downwards by 1 row.
     */
    protected void onScrollDown()
    {
        int iNumItems = m_vMembers.size() ;

        // Make sure we actually can scroll down
        if (    (iNumItems > m_iDisplayableItems) &&
                (m_iTopIndex < (iNumItems - m_iDisplayableItems))) {
            m_iTopIndex++ ;

            // Make sure we don't set initial focus after scrolling.
            m_bSetInitialFocus = true  ;
            surfaceComponents() ;
        }
    }


    /**
     * Verifies that the scroll position is valid and performs adjustments
     * if needed.  This is typically called after a item is removed from
     * the container.
     */
    protected void fixupScrollPosition()
    {
        int iNumItems = m_vMembers.size() ;

        // Is the scroll bar at an invalid position?
        if (m_iTopIndex > (iNumItems - m_iDisplayableItems)) {
            m_iTopIndex = iNumItems - m_iDisplayableItems ;
        }

        // Is our top index below water? (above code could put it there)
        if (m_iTopIndex < 0) {
            m_iTopIndex = 0 ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////
    private class icButtonListener implements PButtonListener
    {
        public icButtonListener()
        {

        }


        /**
         * The specified button has been released
         */
        public void buttonUp(PButtonEvent event)
        {
            m_bSetInitialFocus = true ;

            switch (event.getButtonID()) {
                case PButtonEvent.BID_SCROLL_DOWN:
                    onScrollDown() ;
                    event.consume() ;
                    break ;
                case PButtonEvent.BID_SCROLL_UP:
                    onScrollUp() ;
                    event.consume() ;
                    break ;
            }

            if (!event.isConsumed()) {
                if ((event.getButtonID() >= m_iStartingButtonId) && (event.getButtonID() < m_iStartingButtonId + m_iDisplayableItems)) {
                    int iButtonIndex = event.getButtonID() - m_iStartingButtonId ;

                    // First remove focus from everyone
                    for (int i=0; i<m_surfacedComponents.length; i++) {
                        if ((m_surfacedComponents[i] != null) &&
                                (m_surfacedComponents[i] instanceof PFocusTraversable)) {
                            ((PFocusTraversable) m_surfacedComponents[i]).setFocus(false) ;
                        }
                    }

                    // Now reapply focus
                    if ((m_surfacedComponents[iButtonIndex] != null) &&
                            (m_surfacedComponents[iButtonIndex] instanceof PFocusTraversable)) {
                        // Set Focus
                        ((PFocusTraversable) m_surfacedComponents[iButtonIndex]).setFocus(true) ;
                    }
                }
            }
        }


        /** The specified button is being held down */
        public void buttonRepeat(PButtonEvent event) { }

        /** The specified button has been press downwards */
        public void buttonDown(PButtonEvent event)
        {
            m_bSetInitialFocus = true ;
        }
    }


    /**
     * This class monitors focus and polices the 'only on component may have
     * focus at any time 'rule'.
     */
    private class icFocusMonitor implements PFocusListener
    {
        private PFocusTraversable m_focusHolder ;

        public PFocusTraversable getFocusHolder()
        {
            return m_focusHolder ;
        }


        public void focusGained(PFocusEvent e)
        {
            PFocusTraversable newHolder = (PFocusTraversable) e.getSource() ;

            if (newHolder != m_focusHolder) {
                if (m_focusHolder != null)
                    m_focusHolder.setFocus(false) ;
                m_focusHolder = newHolder;
            }
            //the folling code forces this component to be visible
            int index = indexOf((Component)m_focusHolder);
            int visibleIndexBegin = getFirstVisibleIndex();
            int visibleIndexEnd =  visibleIndexBegin +  getVisibleRowCount()-1;
            if( index != -1 ){
                if( (index >= visibleIndexBegin ) &&
                    (index <= visibleIndexEnd) ){
                    //do nothing
                }else if ( index < visibleIndexBegin){
                    for( int i = 0; i < (visibleIndexBegin-index); i++ ){
                        onScrollUp();
                    }
                }else if ( index > visibleIndexEnd ){
                    for( int i = 0; i < (index-visibleIndexEnd); i++ ){
                        onScrollDown();
                    }
                }
            }

        }


        public void focusLost(PFocusEvent e)
        {
            if (e.getSource() == m_focusHolder)
                m_focusHolder = null ;
        }
    }
}
