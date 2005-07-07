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
 * The item event delivered when a text item's state changes. Look  
 * at the item object and state change to determine what the 
 * state change was and react accordingly.  Querying the state of an item object
 * directly is discouraged as there is a race condition when the object state
 * is changed again.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PTextEvent extends PEvent
{

//////////////////////////////////////////////////////////////////////////////
// Construction
////       
    /** 
     * Create a PTextEvent object with the specified source object.
     *
     * @param objSource The source object where the text component changed.
     */
    public PTextEvent(Object objSource)
    {
        super(objSource, TYPE_TEXT_EVENT) ;
    }
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////   

}