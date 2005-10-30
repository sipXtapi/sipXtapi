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

import java.io.IOException;
import java.io.Writer;

import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.common.CoreContext;

public abstract class DataSetGenerator {

    private static final DocumentFactory FACTORY = DocumentFactory.getInstance();
    private CoreContext m_coreContext;

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
    
    public Document generate() {
        Element items = createItemsElement(getType());
        addItems(items);
        return items.getDocument();
    }
    
    protected abstract DataSet getType();

    protected abstract void addItems(Element items);    
    
    /**
     * Writes document to specified writer
     * 
     * @param writer
     * @throws IOException
     */
    public void write(Writer writer) throws IOException {
        Document document = generate();
        document.write(writer);
    }
}
