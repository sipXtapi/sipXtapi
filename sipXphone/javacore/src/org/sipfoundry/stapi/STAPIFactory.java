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
 
 
package org.sipfoundry.stapi ;

import org.sipfoundry.stapi.event.* ;
import javax.telephony.* ;
import org.sipfoundry.sipxphone.sys.app.* ;


/**
 * This class should not and will not be exposed to developers
 */
public class STAPIFactory
{
    protected static STAPIFactory  m_instance ;
    protected PCallManager  m_callManager ;
    protected PMediaManager m_mediaManager ;

    /**
     * protected constructor guards against user construction
     */
    private STAPIFactory()
    {
        m_callManager = new PCallManager() ;
        m_mediaManager = new PMediaManager() ;
    }
    
    
    public static STAPIFactory getInstance()
    {
        if (m_instance == null) {
            m_instance = new STAPIFactory() ;
        }
        
        return m_instance ;
    }
    
    public PCallManager getCallManager()
    {
        return m_callManager ;
    }
    
    
    public PMediaManager getMediaManager()
    {
        return m_mediaManager ;
    }
}
