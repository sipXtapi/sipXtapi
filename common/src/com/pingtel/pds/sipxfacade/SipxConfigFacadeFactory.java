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
    
    public static SipxConfigFacade getFacade() {
        if (m_facade == null)
        {
            Exception e = null;
            try {
                String facadeClassname = System.getProperty("sipxconfig.facade", 
                        SipxConfigFacadeImpl.class.getName());
                Class facadeClass = Class.forName(facadeClassname); 
                // TODO: Read from property file, to instantiate
                // real implementation
                m_facade = (SipxConfigFacade)facadeClass.newInstance();
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


