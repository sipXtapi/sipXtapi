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

package com.pingtel.commserver.beans;

import com.pingtel.commserver.utility.PathLocatorUtility;
import com.pingtel.pds.common.PDSException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.XMLOutputter;
import org.jdom.JDOMException;
import org.jdom.Document;

import javax.xml.transform.stream.StreamSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PipedOutputStream;
import java.io.PipedInputStream;

/**
 * superclass for the beans which update the destinations.xml file and produce
 * the resultant configuration XML files.
 *
 * @author ibutcher
 *
 */
public abstract class ProcessDestinationsBean {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////
    public static String DESTINATIONS_FILE = PathLocatorUtility.
            getPath( PathLocatorUtility.XML_FOLDER ) + "destinations.xml";

    private static final String MAPPING_RULES2_XLST = PathLocatorUtility.
            getPath( PathLocatorUtility.XML_FOLDER ) + "mappingrules2.xslt";
    private static final String MAPPING_RULES2_XML = PathLocatorUtility.
            getPath( PathLocatorUtility.XML_FOLDER ) + "mappingrules2.xml.in";
    private static final String AUTH_RULES_XSLT = PathLocatorUtility.
            getPath( PathLocatorUtility.XML_FOLDER ) + "authrules.xslt";
    private static final String AUTH_RULES_XML = PathLocatorUtility.
            getPath( PathLocatorUtility.XML_FOLDER ) + "authrules.xml.in";
    private static final String TEMP_XML = PathLocatorUtility.
            getPath( PathLocatorUtility.XML_FOLDER ) + "temp.xml";

    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    protected SAXBuilder mSAXbuilder = new SAXBuilder();
    //protected XMLOutputter mXMLOutputter = new XMLOutputter("   ", true );

    /**
     * object monitor used to prevent the child bean both writing to the
     * destinations xml file at the same time
     * */
    protected static Object mDestinationsFileLock = new Object();

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////
    public static void generateMappingRules2 (  )
            throws PDSException {

        try {
            generateXMLConfigFile(MAPPING_RULES2_XLST, MAPPING_RULES2_XML );
        }
        // @TODO Add error handling messages
        catch (Exception ex) {
            throw new PDSException(ex.getMessage());
        }
    }


    public static void generateAuthRules (  )
        throws PDSException {

        try {
            generateXMLConfigFile(AUTH_RULES_XSLT, AUTH_RULES_XML );
        }
        // @TODO Add error handling messages
        catch (Exception ex) {
            throw new PDSException(ex.getMessage());
        }
   }


    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////

    // brutal synchronization level, i.e. only on object of all of the subclasses
    // may run this method at the same time.   This shouldn't make too much
    // difference as the child classes are only run very very infrequently.
    private static synchronized void generateXMLConfigFile( String styleSheet,
                                                            String outputConfigFile)
            throws IOException, TransformerException, JDOMException {

        FileOutputStream fout = new FileOutputStream( TEMP_XML );
        StreamSource xsltSource = new
                StreamSource( styleSheet );
        StreamSource xmlSource = new
                StreamSource( ProcessDestinationsBean.DESTINATIONS_FILE );

        StreamResult result = new StreamResult( fout );

        TransformerFactory tFact =
                TransformerFactory.newInstance();
        Transformer transformer =
                tFact.newTransformer( xsltSource );
        transformer.transform( xmlSource, result );
        fout.flush();
        fout.close();
        prettyPrint( outputConfigFile );
    }

    private static synchronized void prettyPrint( String outputConfigFile)
            throws IOException, JDOMException {

        SAXBuilder builder = new SAXBuilder();
        org.jdom.output.Format f = org.jdom.output.Format.getPrettyFormat();        
        XMLOutputter xmlOut = new XMLOutputter( f );
        FileOutputStream fout = new FileOutputStream( outputConfigFile );
        Document doc = builder.build( TEMP_XML );
        xmlOut.output( doc, fout );
        fout.flush();
        fout.close();
    }
    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}
