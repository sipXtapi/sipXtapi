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
 *
 * @author Robert J. Andreasen Jr.
 */
 
public class PTabEvent extends EventObject
{                      
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected int m_iTab ;

//////////////////////////////////////////////////////////////////////////////
// Constructors
////    
    /** 
     *
     */
    public PTabEvent(Object objSource, int iTab)
    {
        super(objSource) ;               
        
        m_iTab = iTab ;
    }

    
//////////////////////////////////////////////////////////////////////////////
// public methods
////    

    /** 
     *
     */
    public int getTab()
    {
        return m_iTab ;   
    }        
}