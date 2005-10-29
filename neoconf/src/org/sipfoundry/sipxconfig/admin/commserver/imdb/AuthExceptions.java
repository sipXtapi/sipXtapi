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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.Iterator;
import java.util.List;

import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingManager;

public class AuthExceptions extends DataSetGenerator {
    private ForwardingManager m_forwardingContext;

    protected DataSet getType() {
        return DataSet.AUTH_EXCEPTION;
    }

    protected void addItems(Element items) {
        List forwardingAliases = m_forwardingContext.getForwardingAuthExceptions();
        for (Iterator i = forwardingAliases.iterator(); i.hasNext();) {
            String exception = (String) i.next();
            Element user = addItem(items).addElement("user");
            user.setText(exception);
        }
    }
    
    public void setForwardingContext(ForwardingManager forwardingContext) {
        m_forwardingContext = forwardingContext;
    }    
}
