/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package com.pingtel.pds.sipxfacade;

import org.apache.xml.utils.WrappedRuntimeException;

/**
 * Comments
 */
public class SipxConfigFacadeFactory {
    
    private static SipxConfigFacade m_facade;
    
    private static boolean m_initialized = false;
    
    public static boolean hasImplementation() {
        return getFacade() != null;
    }    
    
    public static SipxConfigFacade getFacade() {
        if (m_facade == null && m_initialized == false)
        {
            m_initialized = true;
            Exception e = null;
            try {
                String facadeClassname = System.getProperty("sipxconfig.facade");
                if (facadeClassname != null) {
                    Class facadeClass = Class.forName(facadeClassname); 
                    m_facade = (SipxConfigFacade)facadeClass.newInstance();
                }
            }
            catch (ClassNotFoundException cnfe) {
                e = cnfe;
            }
            catch (InstantiationException ie) {
                e = ie;
                
            }
            catch (IllegalAccessException iae) {
                e = iae;                
            }

            if (e != null) {
                throw new WrappedRuntimeException("Cannot create sipxconfig facade", e);
            }
        }
        
        return m_facade;
    }
}


