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


import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.common.RedirectServletException;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.user.ApplicationAdvocate;
import com.pingtel.pds.pgs.user.ApplicationAdvocateHome;
import com.pingtel.pds.pgs.user.ApplicationHome;

import javax.servlet.jsp.JspException;
import java.io.*;
import java.net.URL;
import java.util.Properties;

/**
 * This tag is used to dowload a file from an external source
 * (i.e. my.pingtel.com) to a local directory on sipxchange.
 * As well as install the application in the config server.
 * @author dbrown
 *
 */
public class DownloadApplicationsTag extends StyleTagSupport {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////
    private static String FILE_SEPARATOR = System.getProperty ("file.separator");
    private static final int DOWNLOAD_BUFFER_SIZE = 4096;

    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    private String mSourceURL = null;
    private String mApplicationName = null;
    private String mApplicationDescription = null;
    private String mDeviceTypeID = null;
    private String mRefPropertyID = null;
    private String mShouldCreate = null;

    private ApplicationHome mApplicationHome = null;
    private ApplicationAdvocate mApplicationAdvocateEJBObject = null;


    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////
    /**
     * standard JavaBean method
     */
    public void setSourceurl ( String sourceurl ) {
        mSourceURL = sourceurl;
    }

    /**
     * standard JavaBean method
     */
    public void setApplicationname ( String applicationname ) {
        mApplicationName = applicationname;
    }

    /**
     * standard JavaBean method
     */
    public void setApplicationdescription ( String applicationdescription ) {
        mApplicationDescription = applicationdescription;
    }

    /**
     * standard JavaBean method
     */
    public void setDevicetypeid ( String devicetypeid ){
        mDeviceTypeID = devicetypeid;
    }

    /**
     * standard JavaBean method
     */
    public void setRefpropertyid ( String refpropertyid ) {
        mRefPropertyID = refpropertyid;
    }

    /**
     * standard JavaBean method
     */
    public void setShouldcreate ( String shouldcreate ) {
        mShouldCreate = shouldcreate;
    }

    /**
     * standard tag handler method
     */
    public int doStartTag() throws JspException {

        String localJarName = null;

        try {
            if ( mApplicationHome == null ) {
                mApplicationHome = ( ApplicationHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationHome.class,
                                                                    "Application" );

                ApplicationAdvocateHome applicationAdvocateHome =
                        (ApplicationAdvocateHome)
                            EJBHomeFactory.getInstance().getHomeInterface(
                                    ApplicationAdvocateHome.class,
                                    "ApplicationAdvocate" );

                mApplicationAdvocateEJBObject = applicationAdvocateHome.create();
            }

            String pgsPropertiesFileName =
                    new String ( PathLocatorUtil.getInstance().getPath( PathLocatorUtil.CONFIG_FOLDER, PathLocatorUtil.PGS ) +
                        "pgs.props" );


            File downloadedApplicationsDir =
                    new File ( PathLocatorUtil.getInstance().getPath(
                            PathLocatorUtil.DATA_FOLDER, PathLocatorUtil.PGS ) +
                                "applications" + FILE_SEPARATOR + "downloaded" );

            if ( !downloadedApplicationsDir.exists() ) {
                downloadedApplicationsDir.mkdirs();
            }

            String newJarName = getJarNameFromURL (mSourceURL);

            localJarName =
                    downloadedApplicationsDir.getCanonicalPath() + FILE_SEPARATOR + newJarName;

            downloadNewJar(localJarName);

            Properties pgsProperties = new Properties();
            pgsProperties.load ( new FileInputStream ( pgsPropertiesFileName ) );
            StringBuffer downloadedApplicationURL = new StringBuffer();

            String applicationsDocRoot =
                    pgsProperties.getProperty("publishedHttpApplicationsDocRoot");

            downloadedApplicationURL.append ( applicationsDocRoot );

            if ( !applicationsDocRoot.endsWith("/")) {
                downloadedApplicationURL.append ("/");
            }

            downloadedApplicationURL.append ( "downloaded/" );
            downloadedApplicationURL.append ( newJarName );

            if ( mShouldCreate.equals( "true" ) ) {
                System.out.println ( "doing createApplication for:");
                System.out.println ( "mApplicationName " + mApplicationName );
                System.out.println ( "getOrganizationID() " + getOrganizationID());
                System.out.println ( "mDeviceTypeID " + mDeviceTypeID );
                System.out.println ( "mRefPropertyID " + mRefPropertyID);
                System.out.println ( "downloadedApplicationURL.toString() " + downloadedApplicationURL.toString() );
                System.out.println ( "mApplicationDescription" + mApplicationDescription);

                mApplicationAdvocateEJBObject.createApplication(    mApplicationName,
                                                                    getOrganizationID(),
                                                                    mDeviceTypeID,
                                                                    mRefPropertyID,
                                                                    downloadedApplicationURL.toString(),
                                                                    mApplicationDescription );
            }
        }
        catch(Exception ex) {
            if ( localJarName != null ) {
                File localJar = new File ( localJarName );

                if ( localJar.exists() ) {
                    localJar.delete();
                }
            }
            throw new RedirectServletException (
                    ex.getMessage(), "../popup/downloadapps.jsp", null );
        }

        return SKIP_BODY;
    }

    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        mSourceURL = null;
        mApplicationName = null;
        mApplicationDescription = null;
        mDeviceTypeID = null;
        mRefPropertyID = null;
        mShouldCreate = null;

        super.clearProperties();
    }



    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////
    /**
     * downloadNewJar opens up a URL connection to the URL stored in the
     * mSourceURL bean field and stores it locally in a jar named with the
     * argument value of newJarName
     * @param newJarName name to call the new local jar
     * @throws IOException is thrown for all exceptions.
     */
    private void downloadNewJar(String newJarName) throws IOException {
        InputStream jarInputStream = null;
        FileOutputStream newJarOutputStream = null;

        try {
            URL url = new URL ( mSourceURL );
            jarInputStream = url.openConnection().getInputStream();
            byte[] buffer = new byte[DOWNLOAD_BUFFER_SIZE];
            int read = -1;
            newJarOutputStream = new FileOutputStream(newJarName);

            while ((read = jarInputStream.read(buffer, 0, DOWNLOAD_BUFFER_SIZE)) != -1) {
                newJarOutputStream.write(buffer, 0, read);
            }
        }
        finally {
            if ( jarInputStream != null ) {
                jarInputStream.close();
            }
            if ( newJarOutputStream != null ) {
                newJarOutputStream.close();
            }
        }
    }



    /**
     * extracts the jar name (last token) from a given URL string.
     *
     * @param URL url to get jar name from
     * @return jar name.
     */
    private String getJarNameFromURL ( String URL ) {
        return URL.substring( URL.lastIndexOf( "/" ) + 1 );
    }


    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////

    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}
