/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/ApplicationDetailsTag.java#5 $
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
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.phone.DeviceType;
import com.pingtel.pds.pgs.phone.DeviceTypeHome;
import com.pingtel.pds.pgs.profile.RefDataAdvocate;
import com.pingtel.pds.pgs.profile.RefDataAdvocateHome;
import com.pingtel.pds.pgs.profile.RefProperty;
import com.pingtel.pds.pgs.profile.RefPropertyHome;
import com.pingtel.pds.pgs.user.Application;
import com.pingtel.pds.pgs.user.ApplicationHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.Collection;
import java.util.Iterator;

/**
 * <p>Title: ListDevicesTag</p>
 * <p>Description: Tag Library for the DMS JSP Pages</p>
 * <p>Copyright: Copyright (c) 2002</p>
 * <p>Company: Pingtel Corp</p>
 * @author John P. Coffey
 * @version 1.0
 */

public class ApplicationDetailsTag extends StyleTagSupport {

    private String m_applicationID = null;

    private ApplicationHome m_applicationHome = null;
    private RefPropertyHome m_rpHome = null;
    private DeviceTypeHome m_dtHome = null;
    private RefDataAdvocateHome m_rdaHome = null;

    public void setApplicationid ( String applicationid ) {
        this.m_applicationID = applicationid;
    }


    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            // OPtimization to look up all home interfaces here
            if ( m_applicationHome == null ) {
                m_applicationHome =  (ApplicationHome)
                    EJBHomeFactory.getInstance().getHomeInterface( ApplicationHome.class, "Application" );

                m_rpHome =  (RefPropertyHome)
                    EJBHomeFactory.getInstance().getHomeInterface( RefPropertyHome.class, "RefProperty" );

                m_dtHome =  (DeviceTypeHome)
                    EJBHomeFactory.getInstance().getHomeInterface( DeviceTypeHome.class, "DeviceType" );

                m_rdaHome =  (RefDataAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface( RefDataAdvocateHome.class, "RefDataAdvocate" );
            }

            Element documentRootElement = new Element("application");

            Application application =
                m_applicationHome.findByPrimaryKey(
                                            new Integer ( m_applicationID ) );

            Element appDetails =
                    createElementFromObject("details", application );

            documentRootElement.addContent( appDetails );

            Element deviceTypesElement = new Element ( "devicetypes");
            documentRootElement.addContent( deviceTypesElement );

            Collection deviceTypes = m_dtHome.findAll();

            for ( Iterator i = deviceTypes.iterator(); i.hasNext(); ) {

                DeviceType nextEJBObject = (DeviceType) i.next();

                Element nextElement =
                    createElementFromObject("item", nextEJBObject);

                deviceTypesElement.addContent( nextElement );

                /*Collection appRefProperties =
                    m_rpHome.findByDeviceTypeAndProfileType(
                            nextEJBObject.getID(),
                            PDSDefinitions.PROF_TYPE_APPLICATION_REF);*/

                RefDataAdvocate advocate = m_rdaHome.create();

                Collection appRefProperties =
                    advocate.getRefPropertiesForDeviceTypeAndProfile(
                        nextEJBObject.getID(),
                        new Integer ( PDSDefinitions.PROF_TYPE_APPLICATION_REF ) );


                Element refPropertiesElement = new Element ( "refproperties" );

                nextElement.addContent( refPropertiesElement );

                for ( Iterator k = appRefProperties.iterator(); k.hasNext(); ) {

                    RefProperty rp = (RefProperty) k.next();

                    Element rpElement = new Element ( "refproperty" );
                    Element rpIDElement = new Element ( "id" );
                    rpIDElement.addContent( rp.getID().toString() );
                    Element rpNameElement = new Element ( "name" );
                    rpNameElement.addContent( rp.getName() );
                    rpElement.addContent( rpIDElement );
                    rpElement.addContent( rpNameElement );
                    refPropertiesElement.addContent( rpElement );
                }

            }

            // Sends the text to the browser in either xml or html format
            // depending on an optional debug tag attribute
            outputTextToBrowser ( documentRootElement );
        }
        catch(Exception ex ) {
             throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }


    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        m_applicationID = null;

        super.clearProperties();
    }
}