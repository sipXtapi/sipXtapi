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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.IOException;
import java.io.StringWriter;

import javax.xml.parsers.DocumentBuilderFactory;

import junit.framework.Assert;

import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.QName;
import org.dom4j.io.DOMWriter;

/**
 * Collection of utility function to XMLUnit and DOM4J testing with XMLUnit
 */
public abstract class XmlUnitHelper {
    /**
     * In most cases where we use the xpath it's easier to ignore namespace than to construct
     * proper namespace aware XPatch expression
     * 
     * @param namespaceAware
     */
    static void setNamespaceAware(boolean namespaceAware) {
        DocumentBuilderFactory testDocumentBuilderFactory = XMLUnit
                .getTestDocumentBuilderFactory();
        testDocumentBuilderFactory.setNamespaceAware(namespaceAware);

        DocumentBuilderFactory controlDocumentBuilderFactory = XMLUnit
                .getControlDocumentBuilderFactory();
        controlDocumentBuilderFactory.setNamespaceAware(namespaceAware);
    }

    /**
     * Dumps DOM4J document to Strings.
     * 
     * @param doc DOM4J document
     * @return String containing XML document
     */
    static String asString(Document doc) {
        try {
            StringWriter writer = new StringWriter();
            doc.write(writer);
            return writer.toString();
        } catch (IOException e) {
            e.printStackTrace(System.err);
            Assert.fail(e.getMessage());
            return null;
        }
    }

    /**
     * Dumps DOM4J document to standard error.
     * 
     * @param doc
     */
    static void dumpXml(Document doc) {
        System.err.println(asString(doc));
    }

    /**
     * Asserts that the element in expected namespace URI
     * 
     * @param element dom4jj element
     * @param expectedNamespaceUri URI of the namespace
     */
    static void assertElementInNamespace(Element element, String expectedNamespaceUri) {
        QName name = element.getQName();
        String namespaceURI = name.getNamespaceURI();
        Assert.assertEquals(expectedNamespaceUri, namespaceURI);
    }

    static org.w3c.dom.Document getDomDoc(Document doc) throws Exception {
        DOMWriter writer = new DOMWriter();
        return writer.write(doc);
    }
}
