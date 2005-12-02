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

import java.util.* ;

/**
 * PListEvent is the data object passed along as part of the {@link PListListener} 
 * interface.  Developers can query for the selected row and column (if using
 * a multicolumn list).
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PListEvent extends PEvent 
{    
    protected int m_iSelectedRow ;
    protected int m_iSelectedColumn ; 
    

    /**
     * Constructor, takes source object and selected row and column.
     *
     * @param source Source object.
     * @param iSelectedColumn The column that is selected when the event fires.
     * @param iSelectedRow The row that is selected when the event fires.
     */
    public PListEvent(Object source, int iSelectedColumn, int iSelectedRow)
    {
        super(source) ;
        
        m_iSelectedColumn = iSelectedColumn ;
        m_iSelectedRow = iSelectedRow ;        
    }

    
    /**
     * Return the selected row when the event was fired.
     */
    public int getSelectedRow()
    {
        return m_iSelectedRow ;
    }        
    
    
    /**
     * Return the selected column when the event was fired, or -1 if a single
     * column list.
     */
    public int getSelectedColumn()
    {
        return m_iSelectedColumn ;
    }    
}