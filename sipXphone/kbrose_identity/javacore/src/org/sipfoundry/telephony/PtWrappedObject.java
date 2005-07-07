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

package org.sipfoundry.telephony ;

public class PtWrappedObject
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** reference to the native object */
    protected long m_lHandle = 0 ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     *
     */
    public PtWrappedObject(long lHandle) 
    {
        m_lHandle = lHandle ;
    }
    
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public boolean equals(Object obj)
    {
        return (m_lHandle == ((PtCall) obj).m_lHandle) ;
    }

    
    public int hashcode()
    {
        return (int) m_lHandle ;
    }
    
  
//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////    
    public long getHandle() 
    {
        return m_lHandle ;
    }
}
