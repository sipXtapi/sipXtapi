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
package org.sipfoundry.sipxconfig.admin.forwarding;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Iterator;
import java.util.List;

import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;

public class AliasServiceImpl implements AliasService {
    private static final String ITEM = "item";
    private static final String ITEMS = "items";
    private static final String TYPE = "type";
    private ForwardingContext m_forwardingContext;
    private CallGroupContext m_callGroupContext;

    /**
     * Transforms list of alias mappings into standard alias XML.
     * 
     * @return String containg XML document
     */
    public String getForwardingAliases() {
        DocumentFactory docFactory = DocumentFactory.getInstance();
        Document docAliases = docFactory.createDocument();
        Element items = docAliases.addElement(ITEMS);
        items.addAttribute(TYPE, TYPE_ALIAS);
        List forwardingAliases = m_forwardingContext.getForwardingAliases();
        List callGroupAliases = m_callGroupContext.getAliases();
        forwardingAliases.addAll(callGroupAliases);
        for (Iterator i = forwardingAliases.iterator(); i.hasNext();) {
            AliasMapping alias = (AliasMapping) i.next();
            Element item = items.addElement(ITEM);
            Element identity = item.addElement("identity");
            identity.setText(alias.getIdentity());
            Element contact = item.addElement("contact");
            contact.setText(alias.getContact());
        }
        return docToString(docAliases);
    }

    public String getForwardingAuthExceptions() {
        DocumentFactory docFactory = DocumentFactory.getInstance();
        Document docAliases = docFactory.createDocument();
        Element items = docAliases.addElement(ITEMS);
        items.addAttribute(TYPE, TYPE_AUTHEXCEPTION);
        List forwardingAliases = m_forwardingContext.getForwardingAuthExceptions();
        for (Iterator i = forwardingAliases.iterator(); i.hasNext();) {
            String exception = (String) i.next();
            Element item = items.addElement(ITEM);
            Element user = item.addElement("user");
            user.setText(exception);
        }
        return docToString(docAliases);
    }

    /**
     * Dumps XML document to string
     * 
     * @param document
     * @return string representation of XML document
     */
    private String docToString(Document document) {
        try {
            StringWriter writer = new StringWriter();
            document.write(writer);
            return writer.toString();
        } catch (IOException e) {
            // should not happen - string writer
            throw new RuntimeException(e);
        }
    }

    public void setForwardingContext(ForwardingContext context) {
        m_forwardingContext = context;
    }

    public void setCallGroupContext(CallGroupContext context) {
        m_callGroupContext = context;
    }
}
