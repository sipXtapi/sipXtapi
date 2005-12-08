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

import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.common.CoreContext;

public abstract class DataSetGenerator {

    private static final DocumentFactory FACTORY = DocumentFactory.getInstance();
    private CoreContext m_coreContext;
    private String m_sipDomain;

    protected final Element createItemsElement(DataSet dataSet) {
        Document document = FACTORY.createDocument();
        Element items = document.addElement("items");
        items.addAttribute("type", dataSet.getName());
        return items;
    }

    protected final Element addItem(Element items) {
        return items.addElement("item");
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    protected CoreContext getCoreContext() {
        return m_coreContext;
    }

    public void setSipDomain(String sipDomain) {
        m_sipDomain = sipDomain;
    }

    /**
     * @return SIP domain - if not set uses m_coreContext to retrieve domain
     */
    protected String getSipDomain() {
        if (m_sipDomain == null) {
            m_sipDomain = m_coreContext.getDomainName();
        }
        return m_sipDomain;
    }

    public Document generate() {
        Element items = createItemsElement(getType());
        addItems(items);
        return items.getDocument();
    }

    protected abstract DataSet getType();

    protected abstract void addItems(Element items);
}
