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

package com.pingtel.pds.pgs.jsptags.util;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

import java.util.Map;
import java.util.Iterator;
import java.util.Set;

import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.Source;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.output.XMLOutputter;

import com.pingtel.pds.common.TemplatesCache;
import com.pingtel.pds.common.ElementUtilException;

import java.io.StringReader;
import java.io.StringWriter;

/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2001</p>
 * <p>Company: </p>
 * @author unascribed
 * @version 1.0
 */

public abstract class StyleTagSupport extends ExTagSupport {
    /**
     * This abstract super class only has one mandatory tag -
     * the style sheet as its name would suggest
     */
    private String m_stylesheet;

    /**
     * this attribute is used for debugging only and is optional
     * it is used to bypass the XSLT translation and dump the raw
     * internal XML representation to the web browser
     */
    protected boolean m_outputxml = false;

    /**
     * Setter for the XSLT StyleSheet file name (this does not
     * have the foldeer location.
     */
    public void setStylesheet (String stylesheet) {
        m_stylesheet = stylesheet;
    }

    /**
     * Setter for the XSLT StyleSheet file name (this does not
     * have the foldeer location.
     */
    public void setOutputxml ( String outputxml ) {
        m_outputxml = outputxml.equalsIgnoreCase("true")? true: false;
    }

    /** Constructor */
    public StyleTagSupport() {}

    /** Returns the OrganizationID from the HTTP Session */
    /**
    protected String getOrganizationID() {
        return (String)pageContext.getSession().getAttribute("orgid");
    }  */

    /** Returns the OrganizationID from the HTTP Session */
    private String getStyleSheetFileName() {
        return (String)pageContext.getSession().
                    getAttribute("xsltBaseDir") + m_stylesheet;
    }

    /** Uses reflection to convert the EJB Object to its element form */
    protected Element createElementFromObject ( String elementName,
                                                Object introspectableObject )
        throws ElementUtilException {
        return ElementUtil.getInstance().createElementFromObject(
                elementName, introspectableObject) ;
    }


    /**
     * This is how the text gets displayed to the browser from
     * each of the subclasses
     *
     * @param rootElement
     */
    protected void outputTextToBrowser (Element documentRootElement )
        throws TransformerException, IOException {

        outputTextToBrowser ( documentRootElement, null );
    }


    protected void outputTextToBrowser (    Element documentRootElement,
                                            Map xformerParameters )
        throws TransformerException, IOException {

        String outputText = convertToString(    documentRootElement,
                                                xformerParameters );

        if ( m_outputxml )
            writeHtml( pageContext.getOut(), outputText);
        else
            pageContext.getOut().print( outputText );
    }

    /**
     * Converts the rootElement of the XML document into a string
     * This method uses the 'outputxml' optional tag to determine whether
     * to translate the document via xslt or not
     *
     * @param rootElement
     * @exception ElementUtilException
     */

    private String convertToString ( Element rootElement )
        throws IOException, TransformerException  {

        return convertToString ( rootElement, null );
    }

    private String convertToString ( Element rootElement, Map xformerParameters )
        throws IOException, TransformerException {
        // Make an xml document from the root element
        Document xmlDocument = new Document(rootElement);
        // If we need to see the pre translated XML uncomment this code
        ByteArrayOutputStream resultStream = null;
        try {
            resultStream = new ByteArrayOutputStream();
            // Debugging => bypass translation
            if (!m_outputxml) {

                StringWriter sw = new StringWriter();
                org.jdom.output.XMLOutputter xmlOut
                    = new org.jdom.output.XMLOutputter();

                xmlOut.output( xmlDocument, sw );

                StringReader sr =
                    new StringReader( StringUtil.unescapHTML( sw.toString() ) );

                Source source = new javax.xml.transform.stream.StreamSource(sr);
                Transformer transformer =
                    TemplatesCache.getInstance().
                        newTransformer( getStyleSheetFileName() );

                if ( xformerParameters != null ) {
                    Set s = xformerParameters.entrySet();

                    for ( Iterator i = s.iterator(); i.hasNext(); ) {
                        Map.Entry entry = (Map.Entry) i.next();
                        if( entry.getValue() != null )
                        {
                            transformer.setParameter( (String) entry.getKey(),
                                    entry.getValue() );                            
                        }
                    }
                }

                StreamResult result = new StreamResult ( resultStream );
                transformer.transform( source, result );
            } else { // just XML mode here
                // Produce just XML output
                org.jdom.output.Format f = org.jdom.output.Format.getPrettyFormat();        
                XMLOutputter outputter = new XMLOutputter (f);
                outputter.output( xmlDocument, resultStream );
            }
            return resultStream.toString();
        } finally {
            if (resultStream != null)
                try { resultStream.close(); } catch (Exception ex){}
        }
    }

    /** The list(XXX) tags all do special post processing on the reflectino elements */
   // protected abstract Element postProcessElement( Element element )
    //    throws PostProcessingException;

    /**
     * Called by super class to remove any state in the pooled beans
     */
    protected void clearProperties() {
        m_stylesheet = null;
        m_outputxml = false;
        super.clearProperties();
    }
}
