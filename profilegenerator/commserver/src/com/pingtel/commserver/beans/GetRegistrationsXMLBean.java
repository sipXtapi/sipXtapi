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


import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.common.RedirectServletException;
import com.pingtel.pds.common.https.HttpsUtil;

import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.Date;

/**
 * GetRegistrationsXMLBeanproduces the details page for viewing
 * active registrations
 * @author dbrown
 *
 */
public class GetRegistrationsXMLBean extends ResourceBean {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    private String mStylesheet = null;
    private String mXMLfile = null;
    private String mRedirectURL = null;
    private String mSort = null;
    private String mIndex = null;
    private String mCount = null;
    private String mMax = null;
    private String mLocation = null;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////
    public GetRegistrationsXMLBean () {
        HttpsUtil.init();
    }

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

    public void setSort(String sort) {
        this.mSort = sort;
    }

    public void setIndex(String index) {
        this.mIndex = index;
    }

    public void setCount(String count ) {
        this.mCount = count;
    }

    public void setMax(String max) {
        this.mMax = max;
    }

    public void setLocation(String location) {
        this.mLocation = location;
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
    public String getHtmlpage() throws RedirectServletException, PDSException{
        String xml = "https://" + mLocation +":8091/shared/" + mXMLfile;
        String xslt = null;

        try {
            xslt =
                    PathLocatorUtil.getInstance().getPath(
                            PathLocatorUtil.XSLT_FOLDER, PathLocatorUtil.PGS ) + mStylesheet;
        }
        catch ( FileNotFoundException ex ) {
                throw new PDSException ( "A system exception occurred trying to find " +
                    "the " + mStylesheet + "  file.  Please call Pingtel support", ex );
       }
        return xmlTransform( xml, xslt );
    }

    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////
    private String xmlTransform ( String xml, String xsltFile )
            throws RedirectServletException {

        Date now = new Date();
        long currentTime = now.getTime()/1000;
        ByteArrayOutputStream resultStream;
        resultStream = new ByteArrayOutputStream();

        try {

            InputStream xmlIn = new URL(xml).openStream();
            StreamSource xsltSource = new StreamSource( xsltFile );
            StreamSource xmlSource = new StreamSource( xmlIn );
            StreamResult result = new StreamResult( resultStream );
            TransformerFactory tFact = TransformerFactory.newInstance();
            Transformer transformer = tFact.newTransformer( xsltSource );
            transformer.setParameter( "sort", mSort );
            transformer.setParameter( "now", currentTime + "" );
            transformer.setParameter( "index", mIndex );
            transformer.setParameter( "count", mCount );
            transformer.setParameter( "max", mMax );
            transformer.setParameter( "location", mLocation );
            transformer.transform( xmlSource, result );

            return resultStream.toString();
       }
        catch (IOException iex) {
            return mResourceBundle.getString("REGISTRATION_MSG_1");
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
