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

public class PListDataEvent extends PEvent
{
    public static final int CONTENTS_CHANGED  = 0 ;
    public static final int INTERVAL_ADDED    = 1 ;
    public static final int INTERVAL_REMOVED  = 2 ;
        
    private int m_iType ;
    private int m_iIndex0 ;
    private int m_iIndex1 ;
    
    
    public PListDataEvent(Object source, int type, int index0, int index1)
    {
        super(source) ;
        
        m_iType = type ;
        m_iIndex0 = index0 ;
        m_iIndex1 = index1 ;                
    }


    public int getIndex0()
    {
        return m_iIndex0 ;
    }
    
    
    public int getIndex1()
    {
        return m_iIndex1 ;
    }
    
    
    public int getType()
    {
        return m_iType ;
    }
}