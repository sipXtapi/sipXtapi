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
 
package com.pingtel.pds.common;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Collection;
import java.text.MessageFormat;

import org.jdom.Document;
import org.jdom.Element;

import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

/**
 * ErrorMessageBuilder is a singleton which reads our error messages XML file
 * and provides a method (collateErrorMessages) to query and format the values
 * extracted from the XML file.
 *
 */
public class PropertyGroupLink {


    synchronized public static PropertyGroupLink getInstance () {
        return instance;
    }

    private PropertyGroupLink( String linkXMLName ) throws RuntimeException {

        try {
            m_document =
                m_db.build( getClass().getResourceAsStream( linkXMLName ) );

        }
        catch ( java.io.IOException e ) {
            throw new RuntimeException ( e.toString() );
        }
        catch ( JDOMException ex ) {
            throw new RuntimeException ( ex.toString() );
        }
    }


    public synchronized Element getMappings () {
        Element e = (Element)m_document.getRootElement().clone();
        // XCF-154
        e.detach();
        return e;
    }

    private Document m_document = null;

    private static SAXBuilder m_db = new SAXBuilder();
    private static final String XML_RESOURCE = "/propertygrouplink.xml";
    private static PropertyGroupLink instance =
                                new PropertyGroupLink ( XML_RESOURCE );

}