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
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.user.Application;
import com.pingtel.pds.pgs.user.ApplicationAdvocate;
import com.pingtel.pds.pgs.user.ApplicationAdvocateHome;
import com.pingtel.pds.pgs.user.ApplicationHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;
import java.io.File;

/**
 * Standard tag handler.   DeleteApplicationTag deletes a given Application
 * from the system.
 *
 * @author IB
 */
public class DeleteApplicationTag extends ExTagSupport {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////
    private static String FILE_SEPARATOR = System.getProperty ("file.separator");

    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    private String mApplicationID;

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
    public void setApplicationid ( String applicationid ){
        mApplicationID = applicationid;
    }

    /**
     * standard tag handler method
     */
    public int doStartTag() throws JspException {
        try {
            if ( mApplicationHome == null ) {
                ApplicationAdvocateHome applicationAdvocateHome =
                    (ApplicationAdvocateHome)
                                EJBHomeFactory.getInstance().getHomeInterface(
                                        ApplicationAdvocateHome.class,
                                        "ApplicationAdvocate" );

                mApplicationHome = ( ApplicationHome )
                    EJBHomeFactory.getInstance().getHomeInterface(
                            ApplicationHome.class,
                            "Application" );

                mApplicationAdvocateEJBObject = applicationAdvocateHome.create();
            }
            mApplicationAdvocateEJBObject.deleteApplication( mApplicationID );

            // see if the Application was a downloaded one, if so we need to
            // delete the Application's jar file.
            File downloadedApplicationsDir =
                    new File ( PathLocatorUtil.getInstance().getPath(
                            PathLocatorUtil.DATA_FOLDER, PathLocatorUtil.PGS ) +
                                "applications" + FILE_SEPARATOR + "downloaded" );
            Application application =
                    mApplicationHome.findByPrimaryKey ( new Integer(mApplicationID) );

            String applicationJarName = getJarNameFromURL ( application.getURL() );

            if ( downloadedApplicationsDir.exists() ) {
                File [] directoryList = downloadedApplicationsDir.listFiles();

                for ( int counter = 0; counter < directoryList.length; ++counter ) {
                    if ( directoryList [counter].getName().equals ( applicationJarName ) ) {
                        directoryList [counter].delete();
                        break;
                    }
                }
            }

        }
        catch ( Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }

    /**
     * standard tag handler method
     */
    protected void clearProperties() {
        mApplicationID = null;
        super.clearProperties();
    }


    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////
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
