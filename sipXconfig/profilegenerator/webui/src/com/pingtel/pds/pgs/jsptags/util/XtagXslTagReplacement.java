/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package com.pingtel.pds.pgs.jsptags.util;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.Iterator;
import java.util.Map;

import javax.servlet.jsp.JspException;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;

/**
 * XCF-118 - xtags is fading into oblivian and you can't even find binaries or source for
 * it anymore as this dir appears empty
 *   http://jakarta.apache.org/site/downloads/downloads_taglibs-xtags.html
 * In any event, safest to use same XSL transformation rest of tags use.
 */
public final class XtagXslTagReplacement {
    
    public static String style(String xml, String xslFilename, Map params) throws JspException {
        Reader xslReader = null;
        try {
            xslReader = new FileReader(xslFilename);
            StringReader xmlReader = new StringReader(xml);
            return style(xmlReader, xslReader, params);
        } catch (FileNotFoundException e) {
            throw new JspException("Cannot find xsl file " + xslFilename);
        } finally {
            if (xslReader != null) {
                try {
                    xslReader.close();
                } catch (IOException nonFatal) {}
            }
        }
    }
    
    public static String style(Reader xml, Reader xsl, Map params) throws JspException {
            Source xmlSource = new javax.xml.transform.stream.StreamSource(xml);
            TransformerFactory factory = TransformerFactory.newInstance();
            Source xslSource = new javax.xml.transform.stream.StreamSource(xsl);
            Transformer transformer;
            try {
                transformer = factory.newTransformer(xslSource);
            } catch (TransformerConfigurationException tce) {
                throw new JspException("Error transforming document", tce);
            }
            if (params != null && !params.isEmpty()) {
                Iterator entries = params.entrySet().iterator();
                while (entries.hasNext()) {
                    Map.Entry entry = (Map.Entry) entries.next();
                    transformer.setParameter((String) entry.getKey(), entry.getValue());
                }
            }
            StringWriter sw = new StringWriter();
            StreamResult result = new StreamResult(sw);
            try {
                transformer.transform(xmlSource, result);
            } catch (TransformerException te) {
                throw new JspException("Error transforming document", te);
            }        
            
            return sw.toString();
    }

}
