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

public interface PListDataListener
{
    /** 
     * Sent after the indices in the index0,index1 interval have been inserted in the data model.  
     */
    public void intervalAdded(PListDataEvent e) ;


    /** 
     * Sent after the indices in the index0,index1 interval have been removed from the data model.
     */
    public void intervalRemoved(PListDataEvent e) ;
    
    /** 
     * Sent when the contents of the list has changed in a way that's to complex to characterize with the previous methods.
     */
    public void contentsChanged(PListDataEvent e) ;
}