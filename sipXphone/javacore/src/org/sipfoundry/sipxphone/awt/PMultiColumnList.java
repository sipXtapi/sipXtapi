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

import org.sipfoundry.sipxphone.awt.event.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.service. * ;
import org.sipfoundry.sipxphone.sys. * ;


/**
 * @author Robert J. Andreasen, Jr.
 */
public class PMultiColumnList extends PContainer
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final int   COL1                  = 0 ;
    public static final int   COL2                  = 1 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    protected Vector                  m_vListItems[] = new Vector[2] ;
    protected PItemRenderer           m_itemRenderer[] = new PItemRenderer[2] ;

    protected Component               m_components[][] ;
    protected PScrollbar              m_scrollbar ;
    protected int                     m_iTopIndex = 0 ;
    protected Vector                  m_vListListeners = new Vector() ;

    protected int                     m_iSelectedCol = -1 ;
    protected int                     m_iSelectedRow = -1 ;

    /** Was a button just pressed, if so which one? */
    private int                     m_iButtonDown = -1 ;
    /** responds to tooltip/popup timeouts. */
    private icPopupHandlerHandler   m_popupHandler = new icPopupHandlerHandler() ;


    /**
     *
     */
    public PMultiColumnList()
    {
        m_vListItems[COL1] = new Vector() ;
        m_vListItems[COL2] = new Vector() ;
        createComponents() ;
        layoutComponents() ;
        addButtonListener(new icButtonListener()) ;
        setOpaque(false) ;
    }


    /**
     *
     */
    public void addElement(int iList, Object objItem)
    {
        m_vListItems[iList].addElement(new PListItemHolder(objItem, null)) ;

        int iScrollPositions = Math.max(getCount(COL1), getCount(COL2)) ;

        if (iScrollPositions > 4) {
            m_scrollbar.setPosition(m_iTopIndex) ;
            m_scrollbar.setNumPosition(iScrollPositions-4) ;
        }

        updateRenderers() ;
    }


    /**
     *
     */
    public void addElement(int iList, Object objItem, String strHint)
    {
        m_vListItems[iList].addElement(new PListItemHolder(objItem, strHint)) ;

        int iScrollPositions = Math.max(getCount(COL1), getCount(COL2)) ;

        if (iScrollPositions > 4) {
            m_scrollbar.setPosition(m_iTopIndex) ;
            m_scrollbar.setNumPosition(iScrollPositions-4) ;
        }

        updateRenderers() ;
    }


    /**
     *
     */
    public void removeAllItems()
    {
        m_vListItems[COL1].removeAllElements() ;
        m_vListItems[COL2].removeAllElements() ;

        m_scrollbar.setNumPosition(0) ;
        m_scrollbar.setPosition(0) ;
        m_iTopIndex = 0 ;

        updateRenderers() ;
    }


    public void removeAllItems(int iCol)
    {
        m_vListItems[iCol].removeAllElements() ;

        m_scrollbar.setNumPosition(0) ;
        m_scrollbar.setPosition(0) ;
        m_iTopIndex = 0 ;

        updateRenderers() ;
    }


    /**
     *
     */
    public int getCount(int iCol)
    {
        return m_vListItems[iCol].size() ;
    }


    /**
     *
     */
    public Object getElement(int iCol, int index)
    {
        if (index < getCount(iCol))
            return ((PListItemHolder) m_vListItems[iCol].elementAt(index)).getData() ;
        else
            return null ;
    }


    /**
     *
     */
    public String getElementHintAt(int iCol, int index)
    {
        if (index < getCount(iCol))
            return ((PListItemHolder) m_vListItems[iCol].elementAt(index)).getHint() ;
        else
            return null ;
    }



    /**
     *
     */
    protected void createComponents()
    {
        PDefaultItemRenderer   defRenderer = new PDefaultItemRenderer() ;

        defRenderer.setBkColor(SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND)) ;
        defRenderer.setSelBkColor(SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND)) ;

        m_itemRenderer[COL1] = m_itemRenderer[COL2] = defRenderer ;
        m_scrollbar = new PScrollbar() ;
        m_components = new Component[2][4] ;
    }


    public void doLayout()
    {
        Dimension dimSize = getSize() ;
        int       iWidth  = ((dimSize.width - 18) / 2) ;
        int       iHeight = dimSize.height ;

        m_scrollbar.setBounds(iWidth+3, 0, 14, iHeight-8) ;
        updateRenderers() ;

        super.doLayout() ;
    }


    /**
     *
     */
    public void setSelection(int iCol, int iRow)
    {
        int iOldSelectedCol = m_iSelectedCol ;
        int iOldSelectedRow = m_iSelectedRow ;

        // Don't bother redrawing if already set
        if ( (iCol != iOldSelectedCol) || (iRow != iOldSelectedRow) ) {
            m_iSelectedCol = iCol ;
            m_iSelectedRow = iRow ;

            fireSelectionChange(m_iSelectedCol, m_iSelectedRow) ;

            // A multi column list doesn't actually support selection...
            m_iSelectedRow = -1 ;
            m_iSelectedCol = -1 ;
        }
    }


    /**
     *
     */
    public void clearSelection()
    {
        int iOldSelectedRow = m_iSelectedRow ;
        int iOldSelectedCol = m_iSelectedCol ;

        // Don't bother redrawing if selection is not already set
        if (iOldSelectedRow != -1) {
            m_iSelectedRow = -1 ;
            m_iSelectedCol = -1 ;

            updateRenderers() ;
            fireSelectionChange(m_iSelectedCol, m_iSelectedRow) ;
        }
    }


    protected void handleSelectionChange(int iCol, int iDisplayIndex)
    {
        int iRealIndex = iDisplayIndex + m_iTopIndex ;

        // ignore if out of bounds
        if ( iRealIndex < getCount(iCol)) {
            if ( (iCol == m_iSelectedCol) && (m_iSelectedRow == iDisplayIndex) ) {
                clearSelection() ;
            }
            else {
                setSelection(iCol, iDisplayIndex) ;
            }
        }
    }


    /**
     *
     */
    public void scrollUp(int iByLines)
    {
        if (m_iTopIndex > 0) {

            m_iTopIndex -= iByLines ;
            if (m_iTopIndex < 0)
                m_iTopIndex = 0 ;

            if (m_scrollbar != null) {
                m_scrollbar.setPosition(m_iTopIndex) ;
            }
            updateRenderers() ;
            repaint() ;
        }
    }


    /**
     *
     */
    public void scrollDown(int iByLines)
    {
        int iLowestTop = Math.max(getCount(COL1), getCount(COL2)) - 4 ;

        if (m_iTopIndex < iLowestTop) {

            m_iTopIndex += iByLines ;
            if (m_iTopIndex > iLowestTop)
                m_iTopIndex = iLowestTop ;

            if (m_scrollbar != null) {
                m_scrollbar.setPosition(m_iTopIndex) ;
            }

            updateRenderers() ;
            repaint() ;
        }
    }


    /**
     *
     */
    public void addListListener(PListListener listener)
    {
        m_vListListeners.addElement(listener) ;
    }


    /**
     *
     */
    public void removeListListener(PListListener listener)
    {
        m_vListListeners.removeElement(listener) ;
    }


    /**
     *
     */
    protected void fireSelectionChange(int iNewCol, int iNewRow)
    {
         for (Enumeration e = m_vListListeners.elements() ; e.hasMoreElements() ;) {
            PListListener listener = (PListListener) e.nextElement() ;
            if (listener != null) {
                try {
                    listener.selectChanged(new PListEvent(this, iNewCol, iNewRow + m_iTopIndex)) ;
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


    /**
     *
     */
    public void setItemRenderer(PItemRenderer renderer)
    {
        m_itemRenderer[COL1] = m_itemRenderer[COL2] = renderer ;
        updateRenderers() ;
    }


    /**
     *
     */
    public void setItemRenderer(int iCol, PItemRenderer renderer)
    {
        m_itemRenderer[iCol] = renderer ;
        updateRenderers() ;
    }


    /**
     *
     */
    public PItemRenderer getItemRenderer(int iCol)
    {
        return m_itemRenderer[iCol] ;
    }



    /**
     *
     */
    public void updateRenderers()
    {
        int i ;
        int iNumElements ;

        // Update List 1 Renderers
        iNumElements = getCount(COL1) ;
        for (i=m_iTopIndex; i<m_iTopIndex+4; i++) {
            if (i < iNumElements) {
                layoutComponent(COL1, i-m_iTopIndex, m_itemRenderer[COL1].getComponent(this, getElement(COL1, i), (m_iSelectedCol == COL1) && (m_iSelectedRow==(i-m_iTopIndex)))) ;
            } else {
                layoutComponent(COL1, i-m_iTopIndex, m_itemRenderer[COL1].getComponent(this, null, (m_iSelectedCol == COL1) && (m_iSelectedRow==(i-m_iTopIndex)))) ;
            }
        }

        // Update List 2 Renderers
        iNumElements = getCount(COL2) ;
        for (i=m_iTopIndex; i<m_iTopIndex+4; i++) {
            if (i < iNumElements) {
                layoutComponent(COL2, i-m_iTopIndex, m_itemRenderer[COL2].getComponent(this, getElement(COL2, i), (m_iSelectedCol == COL2) && (m_iSelectedRow==(i-m_iTopIndex)))) ;
            } else {
                layoutComponent(COL2, i-m_iTopIndex, m_itemRenderer[COL2].getComponent(this, null, (m_iSelectedCol == COL2) && (m_iSelectedRow==(i-m_iTopIndex)))) ;
            }
        }
    }


    /**
     *
     */
    protected void layoutComponents()
    {
        setLayout(null) ;
        add(m_scrollbar) ;

    }


    /**
     *
     */
    protected void layoutComponent(int iCol, int iRow, Component component)
    {
        Component oldComponent = m_components[iCol][iRow] ;

        // Only update if needed
        if (oldComponent != component) {

            if (oldComponent != null) {
                remove(oldComponent) ;
            }
            Dimension dimSize = getSize() ;

            int       iWidth  = ((dimSize.width - 18) / 2) ;
            int       iHeight = (dimSize.height) / 4 ;

            m_components[iCol][iRow] = component ;
            m_components[iCol][iRow].setBounds(iCol*iWidth + ((iCol==0)?0:17), iRow*iHeight, iWidth, iHeight) ;

            add(m_components[iCol][iRow]) ;
        }
    }


    public class icButtonListener implements PButtonListener
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

            switch (event.getButtonID()) {
                case PButtonEvent.BID_L1:
                case PButtonEvent.BID_L2:
                case PButtonEvent.BID_L3:
                case PButtonEvent.BID_L4:
                    {
                        // Shortcut: We know that the L1, L2, L3, and L4 IDs are sequential
                        int iDelta = event.getButtonID() - PButtonEvent.BID_L1 ;

                        // Make sure it is within our list
                        if ((m_iTopIndex + iDelta) < getCount(COL1)) {
                            m_iButtonDown = event.getButtonID() ;
                            Timer.getInstance().addTimer(Settings.getInt("TOOLTIP_TIMEOUT_LENGTH", 1000), m_popupHandler, new Integer(event.getButtonID())) ;
                            event.consume() ;
                        }
                    }
                    break ;
                case PButtonEvent.BID_R1:
                case PButtonEvent.BID_R2:
                case PButtonEvent.BID_R3:
                case PButtonEvent.BID_R4:
                    {
                        // Shortcut: We know that the R1, R2, R3, and R4 IDs are sequential
                        int iDelta = event.getButtonID() - PButtonEvent.BID_R1 ;

                        // Make sure it is within our list
                        if ((m_iTopIndex + iDelta) < getCount(COL2)) {
                            m_iButtonDown = event.getButtonID() ;
                            Timer.getInstance().addTimer(Settings.getInt("TOOLTIP_TIMEOUT_LENGTH", 1000), m_popupHandler, new Integer(event.getButtonID())) ;
                            event.consume() ;
                        }
                    }
                    break ;
            }
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

            switch (event.getButtonID()) {

                case PButtonEvent.BID_L1:
                    handleSelectionChange(COL1, 0) ;
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_L2:
                    handleSelectionChange(COL1, 1) ;
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_L3:
                    handleSelectionChange(COL1, 2) ;
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_L4:
                    handleSelectionChange(COL1, 3) ;
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_R1:
                    handleSelectionChange(COL2, 0) ;
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_R2:
                    handleSelectionChange(COL2, 1) ;
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_R3:
                    handleSelectionChange(COL2, 2) ;
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_R4:
                    handleSelectionChange(COL2, 3) ;
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_SCROLL_UP:
                    scrollUp(1) ;
                    event.consume() ;
                    break ;

                case PButtonEvent.BID_SCROLL_DOWN:
                    scrollDown(1) ;
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

    private class icPopupHandlerHandler implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            int iButtonID = ((Integer) event.getObjectParam()).intValue() ;

            switch (iButtonID) {
                case PButtonEvent.BID_L1:
                case PButtonEvent.BID_L2:
                case PButtonEvent.BID_L3:
                case PButtonEvent.BID_L4:
                    {
                        // Shortcut: We know that the L1, L2, L3, and L4 IDs are sequential
                        int iDelta = iButtonID - PButtonEvent.BID_L1 ;

                        // Make sure it is within our list
                        if ((m_iTopIndex + iDelta) < getCount(COL1)) {
                            String strHint = getElementHintAt(COL1, m_iTopIndex + iDelta) ;
                            if (strHint == null)
                                strHint = AppResourceManager.getInstance().getString("hint/core/multilistctrl/select") ;
                            Shell.getInstance().displayHint(strHint) ;
                        }
                    }
                    break ;
                case PButtonEvent.BID_R1:
                case PButtonEvent.BID_R2:
                case PButtonEvent.BID_R3:
                case PButtonEvent.BID_R4:
                    {
                        // Shortcut: We know that the R1, R2, R3, and R4 IDs are sequential
                        int iDelta = iButtonID - PButtonEvent.BID_R1 ;

                        // Make sure it is within our list
                        if ((m_iTopIndex + iDelta) < getCount(COL2)) {
                            String strHint = getElementHintAt(COL2, m_iTopIndex + iDelta) ;
                            if (strHint == null)
                                strHint = AppResourceManager.getInstance().getString("hint/core/multilistctrl/select") ;
                            Shell.getInstance().displayHint(strHint) ;
                        }
                    }
                    break ;
            }
            m_iButtonDown = -1 ;
        }
    }



    /**
     *
     *
     */
    private class PListItemHolder
    {
        protected Object m_objData ;
        protected String m_strHint ;

        public PListItemHolder(Object objData, String strHint)
        {
            m_objData = objData ;
            m_strHint = strHint ;
        }

        public Object getData()
        {
            return m_objData ;
        }

        public String getHint()
        {
            return m_strHint ;
        }
    }
}
