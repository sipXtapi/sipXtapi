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

public class AliasServiceImpl implements AliasService {
    private ForwardingContext m_context;
    
    /**
     * Transforms list of alias mappings into standard alias XML.
     * @return String containg XML document
     */
    public String getForwardingAliases() {
        DocumentFactory docFactory = DocumentFactory.getInstance();
        Document docAliases = docFactory.createDocument();
        Element items = docAliases.addElement("items");
        items.addAttribute("type", "alias");
        List forwardingAliases = m_context.getForwardingAliases();
        for (Iterator i = forwardingAliases.iterator(); i.hasNext();) {
            AliasMapping alias = (AliasMapping) i.next();
            Element item = items.addElement("item");
            Element identity = item.addElement("identity");
            identity.setText(alias.getIdentity());
            Element contact = item.addElement("contact");
            contact.setText(alias.getContact());
        }
        
        try {
            StringWriter writer = new StringWriter();
            docAliases.write(writer);
            return writer.toString();
        } catch (IOException e) {
            // should not happen - string writer
            throw new RuntimeException(e);
        }
    }

    public ForwardingContext getContext() {
        return m_context;
    }
    
    public void setContext(ForwardingContext context) {
        m_context = context;
    }
}
