/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.security;

import org.jboss.ejb.Container;
import org.jboss.ejb.MethodInvocation;
import org.jboss.ejb.plugins.AbstractInterceptor;
import org.jboss.security.SecurityAssociation;
import org.jboss.security.SimplePrincipal;

public class MDBInterceptor extends AbstractInterceptor {

    protected Container container;

    // This must be externalized (and encrypted) in conjunction with
    //the SDS' connection user/pass code.
    String credential = "SDS";
    SimplePrincipal principal = new SimplePrincipal("SDS");

    public MDBInterceptor() {
    }

    public void setContainer(Container container) {
        this.container = container;
    }

    public Container getContainer() {
        return container;
    }

    public void start() throws Exception {
        super.start();
    }

    public Object invokeHome(MethodInvocation mi) throws Exception {
        mi.setPrincipal(principal);
        mi.setCredential(credential);
        SecurityAssociation.setPrincipal(principal);
        SecurityAssociation.setCredential(credential);
        try {
            Object returnValue = getNext().invokeHome(mi);
            return returnValue;
        }
        finally {
        }
    }

    public Object invoke(MethodInvocation mi) throws Exception {
        mi.setPrincipal(principal);
        mi.setCredential(credential);
        SecurityAssociation.setPrincipal(principal);
        SecurityAssociation.setCredential(credential);
        try {
            Object returnValue = getNext().invoke(mi);
            return returnValue;
        }
        finally {
        }
    }
}
