/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.xmlrpc;

import org.apache.xmlrpc.XmlRpcException;

/**
 * This is Runtime exception that mirrors XmlRpcException Runtime exceptions are better suited for
 * proxied interfaces. If methods implementing such interface throw checked exceptions we need to
 * deal with ugly UndeclaredThrowableException beast.
 */
public class XmlRpcRemoteException extends RuntimeException {
    /**
     * The fault code of the exception.
     */
    private final int m_faultCode;

    public XmlRpcRemoteException(XmlRpcException e) {
        super(e.getMessage(), e.getCause());
        m_faultCode = e.code;
    }
    
    public int getFaultCode() {
        return m_faultCode;
    }
}
