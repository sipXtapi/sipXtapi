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
package com.pingtel.pds.pgs.jsptags;


import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;

import javax.servlet.jsp.JspException;
import java.io.File;


/**
 * Returns a list of applications that are available for download / upload
 * from an external source (i.e. my.pingtel.com).
 * @author dbrown
 *
 */
public class ListApplicationsForDownloadTag extends StyleTagSupport {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////
    private static String FILE_SEPARATOR = System.getProperty ("file.separator");


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    private String mSourceXMLURL;
    private SAXBuilder mSAXBuilder = new SAXBuilder();


    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    /**
     * standard JavaBean method
     */
    public void setSourcexmlurl ( String sourcexmlurl ) {
        mSourceXMLURL = sourcexmlurl;
    }

    public int doStartTag() throws JspException {
        try {
            Element root = new Element ( "applications_list" );
            Document applicationsList = mSAXBuilder.build( mSourceXMLURL );
            root.addContent ( applicationsList.getRootElement().detach() );

            Element downloadedApplications = new Element ( "downloaded_applications");
            root.addContent ( downloadedApplications );

            File downloadedApplicationsDir =
                    new File ( PathLocatorUtil.getInstance().getPath(
                            PathLocatorUtil.DATA_FOLDER, PathLocatorUtil.PGS ) +
                                "applications" + FILE_SEPARATOR + "downloaded" );

            if ( downloadedApplicationsDir.exists() ) {
                String [] directoryList = downloadedApplicationsDir.list();

                for ( int counter = 0; counter < directoryList.length; ++counter ) {
                    if ( directoryList [counter].toLowerCase().endsWith( ".jar" ) ) {
                        Element application =
                                new Element ( "application").setText(directoryList[counter]);

                        downloadedApplications.addContent ( application );
                    }
                }
            }

            outputTextToBrowser ( root );
        }
        catch(  Exception ex ) {
            throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }


    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        mSourceXMLURL = null;

        super.clearProperties();
    }

    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////


    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}