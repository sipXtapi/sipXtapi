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

import java.util.* ;
import org.sipfoundry.sipxphone.awt.* ;


/**
 * The PMenuComponent a simple container for action items (displayed as menu
 * items).  This class offers various convenience methods to ease menu
 * creation and manipulation.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PMenuComponent
{
//////////////////////////////////////////////////////////////////////////////
// Member Variables
////
    private Vector m_vItems ;   // The list of menu items

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default no argument constructor.
     */
    public PMenuComponent()
    {
        m_vItems = new Vector() ;
    }


    /**
     * Constructor accepting an initial lost of menu/action items.
     *
     * @param item The initial list of menu/action items.
     */
    public PMenuComponent(PActionItem items[])
    {
        m_vItems = new Vector(items.length) ;
        for (int i=0; i<items.length; i++) {
            m_vItems.addElement(items[i]) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get the size of this menu component.
     *
     * @return The number of menu items in this container.
     */
    public int getSize()
    {
        return m_vItems.size() ;
    }


    /**
     * Get all of the menu items as an array of action items.
     *
     * @return An array of all the menu items.
     */
    public PActionItem[] getItems()
    {
        PActionItem items[] = new PActionItem[getSize()] ;
        m_vItems.copyInto(items) ;
        return items ;
    }


    /**
     * Get the action item at the specified index position.
     *
     * @param iPosition The index position of the desired menu item.
     *
     * @return The menu item at the specified index position.
     */
    public PActionItem itemAt(int iPosition)
    {
        return (PActionItem) m_vItems.elementAt(iPosition) ;
    }


    /**
     * Remove the action item at the specified index position.
     *
     * @param iPosition The index position of the desired menu item.
     */
    public void removeItemAt(int iPosition)
    {
        m_vItems.removeElementAt(iPosition) ;
    }


    /**
     * Remove all of the action items within this menu component.
     */
    public void removeAllItems()
    {
        m_vItems.removeAllElements() ;
    }


    /**
     * Replace the menu item at the specified index position.
     *
     * @param item The new menu/action item.
     * @param iPosition The index position of the desired menu item.
     */
    public void setItemAt(PActionItem item, int iPosition)
    {
        m_vItems.setElementAt(item, iPosition) ;
    }


    /**
     * Insert a new menu item at the specified index position.
     *
     * @param item The new menu/action item.
     * @param iPosition The index position that this item will be inserted at.
     */
    public void insertItemAt(PActionItem item, int iPosition)
    {
        m_vItems.insertElementAt(item, iPosition) ;
    }


    /**
     * Enable or disable the menu item at the specified index position.
     *
     * @param bEnable The enable/disable state.  Specify true to enable
     *        the menu otherwise specify false to disable the menu item.
     * @param iPosition The index position of the desired menu item.
     */
    public void enableItemAt(boolean bEnable, int iPosition)
    {
        PActionItem item = itemAt(iPosition) ;
        item.setEnabled(bEnable) ;
        setItemAt(item, iPosition) ;
    }


    /**
     * Add a menu item to the end of the list.
     *
     * @param item The new menu/action item.
     */
    public void addItem(PActionItem item)
    {
        m_vItems.addElement(item) ;
    }


    /**
     * Enable menu items by their action command.  This is a convenience method
     * that traverses all of the menu items and enables/disables if the action
     * command matches.
     *
     * @param bEnable The enable/disable state.  Specify true to enable
     *        the menu otherwise specify false to disable the menu item.
     * @param strActionCommand The action command of the menu items that
     *        should be enabled or disabled.
     */
    public void enableItemsByAction(boolean bEnable, String strActionCommand)
    {
        for (int i=0;i<getSize(); i++) {
            PActionItem item = itemAt(i) ;
            if (item != null && strActionCommand.equals(item.getActionCommand())) {
                enableItemAt(bEnable, i) ;
            }
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    protected void dump()
    {
        System.out.println("") ;

        for (int i=0; i<getSize(); i++) {
            PActionItem item = itemAt(i) ;
            System.out.println(item.getActionCommand() + ": lbl=" + item.getLabel() + ", hint=" + item.getHint() + ", enabled=" + item.isEnabled()) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner Classes
////
}
