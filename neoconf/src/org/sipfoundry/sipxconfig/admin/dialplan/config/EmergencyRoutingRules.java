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

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.Iterator;

import org.dom4j.Document;
import org.dom4j.Element;

import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRouting;
import org.sipfoundry.sipxconfig.admin.dialplan.RoutingException;

/**
 * EmergencyRoutingRules generates XML markup for e911rules.xml file
 * 
 * 
 */
public class EmergencyRoutingRules extends XmlFile {
    private static final String E911RULES_XML = "e911rules.xml";
    private Document m_document;

    public Document getDocument() {
        return m_document;
    }

    public void generate(EmergencyRouting er) {
        m_document = FACTORY.createDocument();
        Element mappings = m_document.addElement("mappings");
        Element defaultMatch = mappings.addElement("defaultMatch");
        String externalNumber = er.getExternalNumber();
        String address = er.getDefaultGateway().getAddress();
        generateTransform(defaultMatch, externalNumber, address);
        Collection exceptions = er.getExceptions();
        for (Iterator i = exceptions.iterator(); i.hasNext();) {
            RoutingException exception = (RoutingException) i.next();
            generateException(mappings, exception);
        }
    }

    /**
     * Generate markup for a single exception
     * 
     * <code>
     *  <userMatch>
     *      <userPattern>523124234</userPattern>
     *      <userPattern>623124234</userPattern>
     *      <transform>
     *          <user>911</user>
     *          <host>10.1.2.1</host>
     *      </transform>
     *  </userMatch>
     * </code>
     * 
     * @param mappings parent element
     * @param exception bean representing routing exception
     */
    private void generateException(Element mappings, RoutingException exception) {
        Element userMatch = mappings.addElement("userMatch");
        String[] patterns = exception.getPatterns();
        for (int j = 0; j < patterns.length; j++) {
            Element userPattern = userMatch.addElement("userPattern");
            userPattern.setText(patterns[j]);
        }
        String externalNumber = exception.getExternalNumber();
        String address = exception.getGateway().getAddress();
        generateTransform(userMatch, externalNumber, address);
    }

    /**
     * Generate transform element
     * 
     * <code>
     *      <transform>
     *          <user>externalNumber</user>
     *          <host>address</host>
     *      </transform>
     * </code>
     * 
     * @param userMatch
     * @param externalNumber
     * @param address
     */
    private void generateTransform(Element userMatch, String externalNumber, String address) {
        Element transform = userMatch.addElement("transform");
        Element user = transform.addElement("user");
        user.setText(externalNumber);
        Element host = transform.addElement("host");
        host.setText(address);
    }

    /**
     * Writes to file in a specified directory
     * 
     * @param configDirectory configureation directory
     * @throws IOException
     */
    public void writeToFile(String configDirectory) throws IOException {
        File parent = new File(configDirectory);
        writeToFile(parent, E911RULES_XML);
    }
}
