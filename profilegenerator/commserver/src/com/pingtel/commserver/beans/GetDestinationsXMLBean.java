/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/commserver/src/com/pingtel/commserver/beans/GetDestinationsXMLBean.java#4 $
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
import com.pingtel.pds.common.RedirectServletException;

import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import java.io.ByteArrayOutputStream;

/**
 * GetDestinationsXMLBeanproduces the details pages/forms for the Gateway and
 * dialplans for the commserver configuration.
 *
 * @author dbrown
 *
 */
public class GetDestinationsXMLBean {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////
    private static final String XML_BASE_DIR =
            PathLocatorUtility.getPath( PathLocatorUtility.XML_FOLDER );

    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    private String mStylesheet = null;
    private String mXMLfile = null;
    private String mRedirectURL = null;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////
    public GetDestinationsXMLBean () {}

    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////
    public void setStylesheet( String stylesheet ){
        this.mStylesheet = stylesheet;
    }

    public void setXmlfile( String xmlfile ) {
        this.mXMLfile = xmlfile;
    }

    public String getRedirectURL() {
        return mRedirectURL;
    }

    public void setRedirectURL(String redirectURL) {
        this.mRedirectURL = redirectURL;
    }

    /**
     * getHtmlpage transforms the XML file specifed in the bean's mXMLfile
     * field with the XSLT mStylesheet specified in the bean's mStylesheet
     * field.   This is used in the G/W and dialplan details JSPs to
     * produce the HTML for the pages.
     *
     * @return a String containing the html markup for the appropriate
     * details page.
     * @throws RedirectServletException is thrown for all checked
     * exceptions.
     */
    public String getHtmlpage() throws RedirectServletException {
        String xml = XML_BASE_DIR + mXMLfile;
        String xslt = XML_BASE_DIR + mStylesheet;
        return xmlTransform( xml, xslt );
    }

    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////
    private String xmlTransform ( String xmlFile, String xsltFile )
            throws RedirectServletException {

        ByteArrayOutputStream resultStream;
        resultStream = new ByteArrayOutputStream();

        try {
            StreamSource xsltSource = new StreamSource( xsltFile );
            StreamSource xmlSource = new StreamSource( xmlFile );
            StreamResult result = new StreamResult( resultStream );
            TransformerFactory tFact = TransformerFactory.newInstance();
            Transformer transformer = tFact.newTransformer( xsltSource );
            transformer.transform( xmlSource, result );

            return resultStream.toString();
        }
        catch (Exception ex) {
            throw new RedirectServletException(ex.getMessage(), mRedirectURL, null );
        }
        finally {
            if (resultStream != null)
                try { resultStream.close(); } catch (Exception ex){}
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}
