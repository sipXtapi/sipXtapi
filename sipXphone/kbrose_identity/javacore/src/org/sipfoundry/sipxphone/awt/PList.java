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

import java.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.service.* ;

/**
 * PList is a simple list that supports plug in renderers and a simple data
 * model.
 * <br><br>
 * The renderer implemention is a bit cheesy in that we clone renderers for
 * row of data.  This maybe ok, but I should check the swing implementation
 * and see how they do it.
 * <br><br>
 * The list and selection model are built in and act slightly different than
 * normal desktop implementation.
 * - The list always has some sort of selection unless it is empty or
 *  disabled.<br>
 * - Control catches scroll up and scroll down messages and will adjust the
 *   selection by moving it upwards or downwards if not already at the top or
 *   bottom, respectively.  If the selection is at the top or bottom and the
 *   the user tries to scroll up or down then the list will be scrolled
 *   upwards or downwards while maintaining the top or bottom most selection.
 * - The controls fires off a PActionEvent when a selected item is clicked.
 *   This is considered double clicking.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PList extends PContainer
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Action command sent when a user 'double clicks' on an item */
    public static String ACTION_DOUBLE_CLICK   = "action_list_double_click" ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** our list model */
    private PListModel              m_listModel ;

    /** our item renderer */
    private PItemRenderer           m_itemRenderer ;

    /** Renderer Wraps */
    private icRendererWrap          m_renderWraps[] ;

    /** list scroll bar */
    private PScrollbar              m_scrollbar ;
    /** the index at the top of the control / list item being displayed at row 0 */
    private int                     m_iTopIndex = 0 ;
    /** list of list listeners */
    private Vector                  m_vListListeners = new Vector() ;
    /** list of action listeners */
    private Vector                  m_vActionListeners = new Vector() ;
    /** what row is currently selected? */
    private int                     m_iSelectedRow = -1 ;
    /** Was a button just pressed, if so which one? */
    private int                     m_iButtonDown = -1 ;
    /** responds to tooltip/popup timeouts. */
    private icPopupHandlerHandler   m_popupHandler = new icPopupHandlerHandler() ;
    /** object that listens to our data model and updates the GUI as needed */
    private icDataModelSyncListener m_dataModelSync = new icDataModelSyncListener() ;
    /** is selection enabled? */
    private boolean                 m_bSelectionEnabled ;
    /** should we update the gui? */
    private boolean                 m_bDisplayLocked ;

    /** container for our renderers */
    private icRendererContainer     m_rendererContainer ;

    /** number of items to show in the list */
    private int                     m_numItemsToShow = 4 ;

    /** what portion of the buttons it uses up the right-side */
    private int                     m_topButtonIndex = PButtonEvent.BID_R1;

    /** Should the left hand side buttons trigger selection too? */
    private boolean                 m_useLeftButtons = false;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default constructor
     */
    public PList()
    {
        this(new PDefaultListModel()) ;
        setOpaque(false) ;
    }


    /**
     * Default constructor taking a initial list model
     *
     * @param listModel list model for the list control
     */
    public PList(PListModel listModel)
    {
        m_bDisplayLocked = true ;
        addButtonListener(new icButtonListener()) ;
        createComponents() ;
        setItemRenderer(new PDefaultItemRenderer()) ;

        m_bSelectionEnabled = true ;
        setListModel(listModel) ;
        m_bDisplayLocked = false ;
    }


    protected void initRendererWraps()
    {
        m_renderWraps = new icRendererWrap[m_numItemsToShow] ;
        for (int i=0; i<m_numItemsToShow; i++) {
            m_renderWraps[i] = new icRendererWrap() ;
            m_renderWraps[i].setItemRenderer(m_itemRenderer) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * get a reference to the list model currently feeding this list control
     */
    public PListModel getListModel()
    {
        return m_listModel ;
    }


    /**
     * replace the default/current list model with the passed list model
     */
    public void setListModel(PListModel listModel)
    {
        // Are we replacing an old list model?
        if (m_listModel != null) {
            m_listModel.removeListDataListener(m_dataModelSync) ;
        }

        // Record change and plug into synch listener
        m_listModel = listModel ;
        m_listModel.addListDataListener(m_dataModelSync) ;

        // Reset the World
        m_iTopIndex = 0 ;
        m_scrollbar.setPosition(0) ;

        m_scrollbar.setNumPosition(listModel.getSize()-4) ;

        if (listModel.getSize() > 0) {
            m_iSelectedRow = 0 ;
        } else {
            m_iSelectedRow = -1 ;
        }

        displayScrollbar(listModel.getSize() > m_numItemsToShow) ;
        updateRenderers(true) ;
        repaint() ;
    }


    /**
     * Sets the item renderer that is used to display each individual
     * list item.
     *
     * @param renderer The item renderer that will be used to display each
     *        list item.
     *
     * @see org.sipfoundry.sipxphone.awt.PItemRenderer
     * @see org.sipfoundry.sipxphone.awt.PDefaultItemRenderer
     */
    public void setItemRenderer(PItemRenderer renderer)
    {
        m_itemRenderer = renderer ;

        for (int y=0; y<m_numItemsToShow; y++) {
            m_renderWraps[y].setItemRenderer(renderer) ;
        }

        updateRenderers(true) ;
    }


    /**
     * Gets the item renderer that is currently being used to display each
     * individual list item.
     *
     * @return The current item renderer
     *
     * @see org.sipfoundry.sipxphone.awt.PItemRenderer
     * @see org.sipfoundry.sipxphone.awt.PDefaultItemRenderer
     */
    public PItemRenderer getItemRenderer()
    {
        return m_itemRenderer ;
    }


    /**
     * sets which button becomes the top for  selecting the items in the list control.
     * Defaults to PButtonEvent.BID_R1;
     *
     * @return void
     *
     * @see org.sipfoundry.sipxphone.awt.event.PButtonEvent
     */
    public void setTopButton(int buttonID)
    {
        m_topButtonIndex = buttonID;
    }

    /**
     * set the selection in terms of display row.  This will select the
     * renderer at position x regardless of that data or scroll position.
     */
    protected void setSelectedRow(int iRow)
    {

        // Don't bother redrawing if already set
        if (iRow != m_iSelectedRow) {
            m_iSelectedRow = iRow ;

            updateRenderers(true) ;

            fireSelectionChange(getSelectedIndex()) ;
        }
    }


    /**
     * Set the list selection.  If the selected item will be scrolled into
     * view if needed.
     *
     * @param iIndex new selection
     *
     * @exception ArrayIndexOutOfBoundsException thrown if the index is
     *            negative or greater than the list size
     */
    public void setSelectedIndex(int iIndex)
        throws ArrayIndexOutOfBoundsException
    {
//        System.out.println("iIndex = " + iIndex);
//        System.out.println("m_iTopIndex = " + m_iTopIndex);
//        System.out.println("m_iSelectedRow = " + m_iSelectedRow);
        if (iIndex == -1) {
            setSelectedRow(-1) ;
        } else {

            // Make sure the index is valid
            if ((iIndex < 0) || (iIndex >= m_listModel.getSize())) {
                throw new ArrayIndexOutOfBoundsException("index ("+iIndex+")"+" is out of bounds") ;
            }

            // Simple case: the index is not selected
            if ((iIndex >= m_iTopIndex) && (iIndex < m_iTopIndex+m_numItemsToShow)) {

                setSelectedRow(iIndex - m_iTopIndex) ;
            }
            // Reposition select item at top because outside of view
            else {
                m_iTopIndex = iIndex-m_numItemsToShow+1;
                if( m_iTopIndex < 0 )
                    m_iTopIndex = 0;
                handleSelectionChange(iIndex);
                setSelectedRow(iIndex - m_iTopIndex) ;
                updateRenderers(true) ;

            }

            if (m_scrollbar != null) {
                m_scrollbar.setPosition(m_iTopIndex) ;
                m_scrollbar.redrawScrollbar() ;
            }
        }
    }



    /**
     * get the selection in terms of display rows.  This will return the
     * selection at with no regard for scroll position.
     */
    protected int getSelectedRow()
    {
        return m_iSelectedRow ;
    }


    /**
     * get the selection in terms of item index.  Unlike getSelectedRow,
     * this method accounts for scroll position
     */
    public int getSelectedIndex()
    {
        int iRC = -1 ;

        if (m_iSelectedRow != -1) {
            // adjust for scrolling
            iRC = m_iSelectedRow + m_iTopIndex ;
        }
        return iRC ;

    }


    /**
     *
     */
    public Object getSelectedElement()
    {
        Object objRC = null ;
        int    iIndex  ;

        iIndex = getSelectedIndex() ;
        if (iIndex != -1) {
            objRC = m_listModel.getElementAt(iIndex) ;
        }

        return objRC ;
    }


    /**
     *
     */
    public void clearSelection()
    {
        int iOldSelectedRow = m_iSelectedRow ;

        // Don't bother redrawing if selection is not already set
        if (iOldSelectedRow != -1) {
            m_iSelectedRow = -1 ;

            updateRenderers(true) ;

            fireSelectionChange(getSelectedIndex()) ;
        }
    }


    /**
     * scroll the list upwards with absolutely no regard for selection.  This
     * will leave the selected renderer selected regardless of data.  However,
     * a selection change event will still be fired.
     */
    public void scrollUp(int iByLines)
    {
        if (m_iTopIndex > 0) {

            m_iTopIndex -= iByLines ;
            if (m_iTopIndex < 0)
                m_iTopIndex = 0 ;

            m_rendererContainer.scrollUp() ;
            updateRenderers(false) ;
            refreshRendererAt(0) ;

            if (m_scrollbar != null) {
                m_scrollbar.setPosition(m_iTopIndex) ;
            }

            fireSelectionChange(getSelectedIndex()) ;
        }
    }


    /**
     * scroll the list downwards with absolutely no regard for selection.
     * This will leave the selected renderer selected regardless of data.
     * However, a selection change event will still be fired.
     */
    public void scrollDown(int iByLines)
    {
        int iLowestTop = m_listModel.getSize()-4 ;

        if (m_iTopIndex < iLowestTop) {

            m_iTopIndex += iByLines ;
            if (m_iTopIndex > iLowestTop)
                m_iTopIndex = iLowestTop ;

            m_rendererContainer.scrollDown() ;
            updateRenderers(false) ;
            refreshRendererAt(m_numItemsToShow-1) ;

            if (m_scrollbar != null) {
                m_scrollbar.setPosition(m_iTopIndex) ;
            }

            fireSelectionChange(getSelectedIndex()) ;
        }
    }


    /**
     * add an list listener for selection events
     */
    public void addListListener(PListListener listener)
    {
        m_vListListeners.addElement(listener) ;
    }


    /**
     * remove a list listener (selection events)
     */
    public void removeListListener(PListListener listener)
    {
        m_vListListeners.removeElement(listener) ;
    }


    /**
     * add an action listener for double-click events
     */
    public void addActionListener(PActionListener listener)
    {
        m_vActionListeners.addElement(listener) ;
    }


    /**
     * remove a action listener (double-click events)
     */
    public void removeActionListener(PActionListener listener)
    {
        m_vActionListeners.removeElement(listener) ;
    }

    /**
     * sets total items visible for a list
     */
    public void setNumItemsToShow(int num)
    {
        if (num > 1)
            m_numItemsToShow = num;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * create all of our default renderers
     */
    protected void createComponents()
    {
        // Create ScrollBar
        m_scrollbar = new PScrollbar() ;

        // Container for our renderers
        m_rendererContainer = new icRendererContainer() ;
        m_rendererContainer.setOpaque(false) ;
        add(m_rendererContainer) ;

        // The actual renderers
        initRendererWraps() ;
        for (int y=0; y<m_numItemsToShow; y++) {
            m_rendererContainer.add(m_renderWraps[y]) ;
        }
    }



    /**
     * fire a selection change event off to all interested parties.
     */
    protected void fireSelectionChange(int iNewRow)
    {
         for (Enumeration e = m_vListListeners.elements() ; e.hasMoreElements() ;) {
            PListListener listener = (PListListener) e.nextElement() ;
            if (listener != null) {
                try {
                    listener.selectChanged(new PListEvent(this, -1, iNewRow)) ;
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
         }
     }


    /**
     * fire an action event off to all interested parties.
     */
    protected void fireActionEvent(int iRow)
    {
        for (Enumeration e = m_vActionListeners.elements() ; e.hasMoreElements() ;) {
            PActionListener listener = (PActionListener) e.nextElement() ;
            if (listener != null) {
                try {
                    PActionEvent event = new PActionEvent(this, ACTION_DOUBLE_CLICK) ;
                    event.setIntParam(iRow) ;
                    listener.actionEvent(event) ;
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


    /**
     * Developers can choose whether they would like to enable selection (ability to
     * select items with the scroll wheel and right soft buttons) dynamically.
     *
     * @param bEnable true to enable selection or false to disable
     */
    public void enableSelection(boolean bEnable)
    {
        if (m_bSelectionEnabled != bEnable) {
            m_bSelectionEnabled = bEnable ;
            updateRenderers(true) ;
        }
    }


    /**
     * Update the data and state associated with each of our renderers.
     *
     * @param bShouldUpdateGUI true if we should update the user interface
     *        by calling refreshRenderers or false to purely update renderer
     *        state
     */
    protected void updateRenderers(boolean bShouldUpdateGUI)
    {
        int i ;
        int iNumElements ;

        // Update List Renderers
        if ((m_listModel != null) && (m_renderWraps != null)) {
            iNumElements = m_listModel.getSize() ;
            for (i=m_iTopIndex; i<m_iTopIndex+m_numItemsToShow; i++) {
                if (i < iNumElements) {
                    m_renderWraps[i-m_iTopIndex].setDataAndState(m_listModel.getElementAt(i), m_bSelectionEnabled && (m_iSelectedRow==(i-m_iTopIndex)), bShouldUpdateGUI) ;
                } else {
                    m_renderWraps[i-m_iTopIndex].setDataAndState(null, false, false) ;
                }
            }
        }

        /*
        if ((bShouldUpdateGUI) && !m_bDisplayLocked) {
            refreshRenderers() ;
        }
        */
    }


    public void refreshRendererAt(int iIndex)
    {
        m_renderWraps[iIndex].repaint() ;
    }


    public void refreshRenderers()
    {
        m_rendererContainer.repaint() ;
    }


    public void doLayout()
    {
        displayScrollbar(m_listModel.getSize() > m_numItemsToShow) ;

        super.doLayout() ;
    }



    public PList getList()
    {
        return this ;
    }



    /**
     * handle any sort of selection change
     */
    protected void handleSelectionChange(int iDisplayIndex)
    {
        int iRealIndex = iDisplayIndex + m_iTopIndex ;

        // ignore if out of bounds
        if ( iRealIndex < m_listModel.getSize()) {
            if (m_iSelectedRow == iDisplayIndex) {
                fireActionEvent(getSelectedIndex()) ;
            }
            else {
                setSelectedRow(iDisplayIndex) ;
            }
        }
    }

    /**
     * Shows the scrollbar
     */
    public void showScrollbar()
    {
        displayScrollbar(true);
    }

    /**
     * Show or hide the scrollbar.  This method will muck with the layout
     * and show or hide the scrollbar as appropriate.  This will of course
     * also resize the display area to accommodate any gained or lost
     * screen real-estate.
     *
     * @param bShow true to show the scroll bar, false to hide it.
     */
    private void displayScrollbar(boolean bShow)
    {
        Dimension dimSize = getSize() ;
        boolean bIsShowing = false ;

        // Figure out if the scrollbar is added to the container
        Component[] components = getComponents() ;
        for (int i=0;i<components.length; i++) {
            if (components[i] == m_scrollbar)
                bIsShowing = true ;
                break ;
        }

        // Enable / Show the Scrollbar
        if (bShow) {
            Rectangle rectDesiredBounds = new Rectangle(3, 2, 12, dimSize.height-3) ;
            if (!bIsShowing) {
                add(m_scrollbar) ;
                m_scrollbar.setBounds(rectDesiredBounds) ;
                m_scrollbar.doLayout() ;
                repaint() ;
            } else {
                // even if already shown, make sure the bounds are as desired.
                Rectangle rectBounds = m_scrollbar.getBounds() ;
                if (!rectBounds.equals(rectDesiredBounds)) {
                    m_scrollbar.setBounds(rectDesiredBounds) ;
                    m_scrollbar.doLayout() ;
                    repaint() ;
                }
            }

            Rectangle rect = m_rendererContainer.getBounds() ;
            m_rendererContainer.setBounds(12, 0, dimSize.width-12, dimSize.height) ;
            m_rendererContainer.doLayout() ;

        }
        // Disable / Hide the Scrollbar
        else {
            if (bIsShowing) {
                remove(m_scrollbar) ;
                repaint() ;
            }

            Rectangle rect = m_rendererContainer.getBounds() ;
            m_rendererContainer.setBounds(0, 0, dimSize.width, dimSize.height) ;
            m_rendererContainer.doLayout() ;
        }
    }


    /**
     * The Data Model Sync Listener sits around and listens to notifications
     * form the connected data model.  The listener then directs GUI updates
     * based on those model change.  This allows the user interface to
     * dynamically update and react to changes in the data model.
     */
    private class icDataModelSyncListener implements PListDataListener
    {
        private boolean m_bDisplayingScrollbar ;

        /**
         * Default No Argument Constructor
         */
        public icDataModelSyncListener()
        {
            m_bDisplayingScrollbar = false ;
        }


        /**
         * An interval was added/inserted into the data model
         */
        public void intervalAdded(PListDataEvent e)
        {
            int iIndex0 = e.getIndex0() ;
            int iIndex1 = e.getIndex1() ;

            if (isPastViewport(iIndex0) && isPastViewport(iIndex1)) {
                /*
                 * We don't care about anything added after the view port
                 */
            } else {
                adjustTopIndex() ;
                updateRenderers(false) ;
                repaint() ;
            }

            m_scrollbar.setNumPosition(m_listModel.getSize()-m_numItemsToShow) ;
            boolean bShouldShowScrollbar = (m_listModel.getSize() > m_numItemsToShow) ;
            displayScrollbar(bShouldShowScrollbar) ;
        }


        /**
         * An interval was deleted from the data model
         */
        public void intervalRemoved(PListDataEvent e)
        {
            int iIndex0 = e.getIndex0() ;
            int iIndex1 = e.getIndex1() ;

            if (isPastViewport(iIndex0) && isPastViewport(iIndex1)) {
                /*
                 * We don't care about anything removed after the view port
                 */
            } else {
                adjustTopIndex() ;
                updateRenderers(false) ;
                repaint() ;
            }

            m_scrollbar.setNumPosition(m_listModel.getSize()-m_numItemsToShow) ;
            boolean bShouldShowScrollbar = (m_listModel.getSize() > m_numItemsToShow) ;
            displayScrollbar(bShouldShowScrollbar) ;
        }


        /**
         * Some data was changed in our data model
         */
        public void contentsChanged(PListDataEvent e)
        {
            int iIndex0 = e.getIndex0() ;
            int iIndex1 = e.getIndex1() ;

            if ((isPastViewport(iIndex0) && isPastViewport(iIndex1)) ||
                    (isBeforeViewport(iIndex0) && isBeforeViewport(iIndex1))) {
                /*
                 * We don't care if changes are both below or past our view port window
                 */
            } else {
                adjustTopIndex() ;
                updateRenderers(false) ;
                repaint() ;
            }

            m_scrollbar.setNumPosition(m_listModel.getSize()-m_numItemsToShow) ;
            boolean bShouldShowScrollbar = (m_listModel.getSize() > m_numItemsToShow) ;
            displayScrollbar(bShouldShowScrollbar) ;
        }


        private boolean isPastViewport(int iIndex)
        {
            return (iIndex > m_iTopIndex+4) ;
        }


        private boolean isBeforeViewport(int iIndex)
        {
            return (iIndex < m_iTopIndex) ;
        }


        private void adjustTopIndex()
        {
            int iDataSize = m_listModel.getSize() ;

            if ((m_iSelectedRow == -1) && (iDataSize > 0)) {
                m_iSelectedRow = 0 ;
                fireSelectionChange(m_iSelectedRow) ;
            }

            if (iDataSize < m_numItemsToShow)
                m_iTopIndex = 0 ;
            else {
                if (m_iTopIndex > (iDataSize - m_numItemsToShow))
                    m_iTopIndex = iDataSize - m_numItemsToShow ;
            }

//            System.out.println("iDataSize: " + iDataSize);
//            System.out.println("mSelectedRow: " + m_iSelectedRow);

            if ((m_iSelectedRow >= iDataSize) || (iDataSize == 0)) {
                if (iDataSize > 0) {
//                    m_iSelectedRow = iDataSize % m_numItemsToShow ;
                    m_iSelectedRow = iDataSize-1 ;
                    fireSelectionChange(m_iSelectedRow) ;
                } else {
                    m_iSelectedRow = -1 ;
                    fireSelectionChange(m_iSelectedRow) ;
                }
            }
        }
    }


    private class icButtonListener implements PButtonListener
    {
        /**
         * The specified button has been press downwards
         */
        public void buttonDown(PButtonEvent event)
        {
            // avoid multiple popup requests
            if (m_iButtonDown != -1) {
                return ;
            }

            for (int i = 0; i < 4; i++)
            {
                if (isButtonPushed(event.getButtonID(), i))
                {
                    if ((m_iTopIndex + i) < m_listModel.getSize() && m_numItemsToShow > i) {
                        m_iButtonDown = event.getButtonID() ;
                        Timer.getInstance().addTimer(Settings.getInt("TOOLTIP_TIMEOUT_LENGTH", 1000), m_popupHandler, new Integer(m_iTopIndex+i)) ;
                        event.consume() ;
                        return ;
                    }
                }
            }
        }

        private boolean isButtonPushed(int eventButtonId, int index)
        {
            if (eventButtonId == m_topButtonIndex + index)
                return true;

            if (m_useLeftButtons)
            {
                int lbutton = PButtonEvent.BID_L1 - (m_topButtonIndex - PButtonEvent.BID_R1) + index;
                if (eventButtonId == lbutton)
                    return true;
            }

            return false;
        }

        /**
         * The specified button has been released
         */
        public void buttonUp(PButtonEvent event)
        {
            // Release any popup timers
            if (m_iButtonDown != -1) {
                Timer.getInstance().removeTimers(m_popupHandler) ;
                m_iButtonDown = -1 ;
            }

            for (int i = 0; i < 4; i++)
            {
                if (isButtonPushed(event.getButtonID(), i))
                {
                    if (m_numItemsToShow > i)
                    {
                        handleSelectionChange(i) ;
                        event.consume() ;
                        return ;
                    }
                }
            }

            if (event.getButtonID() == PButtonEvent.BID_SCROLL_UP)
            {
                    if (getSelectedRow() > 0) {
                        setSelectedRow(getSelectedRow()-1) ;
                    } else {
                        scrollUp(1) ;
                    }
                    event.consume() ;
            }
            else
            if (event.getButtonID() == PButtonEvent.BID_SCROLL_DOWN)
            {
                    if ((getSelectedRow() < m_numItemsToShow-1) && ((getSelectedIndex()+1) < m_listModel.getSize())) {
                        setSelectedRow(getSelectedRow()+1) ;
                    } else {
                        scrollDown(1) ;
                    }
                    event.consume() ;
            }
        }


        /**
         * The specified button is being held down
         */
        public void buttonRepeat(PButtonEvent event)
        {
            if (event.getButtonID() == PButtonEvent.BID_SCROLL_UP ||
                event.getButtonID() == PButtonEvent.BID_SCROLL_DOWN)
            {
                        buttonUp(event) ;
            }
        }
    }


    private class icPopupHandlerHandler implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            int iIndex = ((Integer) event.getObjectParam()).intValue() ;

            // Only bother if we have some data...
            if ((iIndex >= 0) && (iIndex < m_listModel.getSize())) {

                if (iIndex == getSelectedIndex()) {
                    String strExtraData = m_listModel.getElementPopupTextAt(iIndex) ;
                    if (strExtraData != null) {
                        Shell.getInstance().displayHint(strExtraData) ;
                    }
                } else {
                    Shell.getInstance().displayHint
                        (getString("hint/core/listctrl/select")) ;
                }
            }
            m_iButtonDown = -1 ;
        }
    }


    /**
     * Container that holds all of our renderers
     */
    private class icRendererContainer extends PContainer
    {
        public icRendererContainer()
        {
            this.setLayout(null) ;
            this.setOpaque(false) ;
        }


        public void doLayout()
        {
            Dimension dimSize = this.getSize() ;
            int       iHeight = (dimSize.height) / m_numItemsToShow ;
            int       iWidth = dimSize.width -2 ;

            for (int y=0; y<m_numItemsToShow; y++) {
                m_renderWraps[y].setBounds(1, y*iHeight, iWidth, iHeight) ;
                m_renderWraps[y].doLayout() ;
            }
            super.doLayout() ;
        }


/*
   [0]
---       U P   D N
0  [1]    [3]   [1]
1  [2]    [0]   [2]
2  [3]    [1]   [3]
3  [4]    [2]   [0]
---
   [5]
*/

        public void setBounds()
        {
            Dimension dimSize = this.getSize() ;
            int       iHeight = (dimSize.height) / m_numItemsToShow ;
            int       iWidth = dimSize.width -2 ;

            for (int y=0; y<m_numItemsToShow; y++) {
                m_renderWraps[y].setBounds(1, y*iHeight, iWidth, iHeight) ;
            }
        }


        public void scrollUp()
        {
            icRendererWrap wrap = m_renderWraps[m_numItemsToShow-1] ;

            for (int i=m_numItemsToShow-2; i>=0; i--) {
                m_renderWraps[i+1] = m_renderWraps[i]  ;
                if (m_renderWraps[i+1] != null)
                    m_renderWraps[i+1].setState(false, false) ;
            }


            m_renderWraps[0] = wrap ;
            if (m_renderWraps[0] != null) {
                m_renderWraps[0].setDataAndState(null, false, false) ;
            }

            setBounds() ;
        }



        public void scrollDown()
        {
            icRendererWrap wrap = m_renderWraps[0] ;

            for (int i=0; i< m_numItemsToShow-1; i++) {
                m_renderWraps[i] = m_renderWraps[i+1]  ;
                if (m_renderWraps[i] != null)
                    m_renderWraps[i].setState(false, false) ;
            }

            m_renderWraps[m_numItemsToShow-1] = wrap ;
            if (m_renderWraps[m_numItemsToShow-1] != null) {
                m_renderWraps[m_numItemsToShow-1].setDataAndState(null, false, false) ;
            }

            setBounds() ;
        }
    }


    /**
     * This inner class acts as a cache for the selected and unselected
     * components.  Typically, i.e. swing, the component would be regotten
     * on selection changes.
     */
    private class icRendererWrap extends PContainer
    {
        PItemRenderer   m_itemRenderer = null ;
        Component       m_renderedComponent = null ;
        Component       m_renderedSelectedComponent = null ;
        Object          m_objData ;
        boolean         m_bSelected ;


        /**
         * Default no argument constructor
         */
        public icRendererWrap()
        {
            m_bSelected = false ;
            this.setLayout(new GridLayout(1,1)) ;
            this.setOpaque(false) ;
            this.enableDoubleBuffering(true) ;
        }


        public void setDataAndState(Object objData, boolean bSelected, boolean bForceRepaint)
        {
            if ((bSelected != m_bSelected) || (objData != m_objData)) {
                removeComponent(m_bSelected) ;
                m_bSelected = bSelected ;

                if (objData != m_objData) {
                    // Clear our cache
                    m_renderedComponent = null ;
                    m_renderedSelectedComponent = null ;
                    m_objData = objData ;
                }
                addComponent(m_bSelected) ;

                if (bForceRepaint) {
                    this.repaint() ;
                }
            }
        }



        /**
         * Set the selection state for this component
         */
        public void setState(boolean bSelected, boolean bForceRepaint)
        {
            if (bSelected != m_bSelected) {
                removeComponent(m_bSelected) ;
                m_bSelected = bSelected ;
                addComponent(m_bSelected) ;

                if (bForceRepaint) {
                    this.repaint() ;
                }
            }
        }


        /**
         * Set the data object for this component
         */
        public void setData(Object objData)
        {
            if (objData != m_objData) {
                removeComponent(m_bSelected) ;
                m_objData = objData ;

                // Clear our cache
                m_renderedComponent = null ;
                m_renderedSelectedComponent = null ;

                addComponent(m_bSelected) ;

                //this.repaint() ;
            }
        }


        /**
         * Set the item renderer for this component
         */
        public void setItemRenderer(PItemRenderer renderer)
        {
            m_itemRenderer = renderer ;

            // Remove the component
            removeComponent(m_bSelected) ;

            // Clear our cache
            m_renderedComponent = null ;
            m_renderedSelectedComponent = null ;

            // Add the component
            addComponent(m_bSelected) ;

            this.repaint() ;
        }


        /**
         * Remove the rendered component from this container.
         */
        protected void removeComponent(boolean bSelected)
        {
            if (bSelected) {
                if (m_renderedSelectedComponent != null) {
                    this.remove(m_renderedSelectedComponent) ;
                }
            } else {
                if (m_renderedComponent != null) {
                    this.remove(m_renderedComponent) ;
                }
            }
        }


        /**
         * Add either the selected or unselected component to our container
         * depending on the specified selected flag.  The component will be
         * created if it doesn't already exist.
         */
        protected void addComponent(boolean bSelected)
        {
            if (bSelected) {
                // Get item if needed
                if ((m_renderedSelectedComponent == null) && (m_objData != null)) {
                    m_renderedSelectedComponent = m_itemRenderer.getComponent(getList(), m_objData, bSelected) ;
                }

                // go ahead and add
                if (m_renderedSelectedComponent != null) {
                    this.add(m_renderedSelectedComponent) ;
                    this.doLayout() ;
                    m_renderedSelectedComponent.doLayout() ;
                }
            } else {
                // Get item if needed
                if ((m_renderedComponent == null) && (m_objData != null)) {
                    m_renderedComponent = m_itemRenderer.getComponent(getList(), m_objData, bSelected) ;

                }
                // go ahead and add
                if (m_renderedComponent != null) {
                    this.add(m_renderedComponent) ;
                    this.doLayout() ;
                    m_renderedComponent.doLayout() ;
                }
            }
        }
    }
}
