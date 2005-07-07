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

package org.sipfoundry.sipxphone.awt.event ;

/**
 * The item event delievered when an item's state changes. Developers 
 * should look at the item object and state change to determine what the 
 * state change was and react accordingly. Querying the state of item object
 * directly is discouraged as there is a race condition when the object state
 * is changed again.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PItemEvent extends PEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Object m_objItem ;
    private int    m_iState ;

//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** PItem object state: selected */
    public static final int SELECTED  = 1 ;
    /** PItem object state: deselected */
    public static final int DESELECTED = 0 ;    
       

//////////////////////////////////////////////////////////////////////////////
// Construction
////       
    /** 
     * Create a PItemEvent object with the specified source object, item 
     * object and state value. 
     *
     * @param objSource
     * @param objItem
     * @param iState 
     *
     */
    public PItemEvent(Object objSource, Object objItem, int iState)
    {
        super(objSource, TYPE_ITEM_EVENT) ;
        
        m_objItem = objItem ;
        m_iState = iState ;                        
    }
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////   

    /**
     * Get the item's new state.
     *
     * @return The new event state.
     */
    public int getStateChange()
    {
        return m_iState ;
        
    }
    
    
    /**
     * Get the source item object.
     *
     * @return The source item object.
     */
    public Object getItem()
    {
        return m_objItem ;        
    }
}