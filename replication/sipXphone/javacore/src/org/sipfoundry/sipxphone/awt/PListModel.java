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

public interface PListModel
{
    /**
     * How many elements do we have in our list?
     */
    public int getSize() ;
    
    
    /**
     * snag the element at position iIndex
     */
    public Object getElementAt(int iIndex) ;


    /**
     * snag any popup information related to the item at position iIndex
     * or null if nothing is available.
     */
    public String getElementPopupTextAt(int iIndex) ;
    
    
    /**
     * stock list data listener interface
     */
    public void addListDataListener(PListDataListener listener) ;


    /**
     * stock list data listener interface
     */    
    public void removeListDataListener(PListDataListener listener) ;
}
