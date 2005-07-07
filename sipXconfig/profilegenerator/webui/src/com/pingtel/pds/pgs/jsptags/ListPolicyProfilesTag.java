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
import com.pingtel.pds.common.PostProcessingException;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.phone.DeviceGroup;
import com.pingtel.pds.pgs.phone.DeviceGroupHome;
import com.pingtel.pds.pgs.phone.DeviceHome;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserHome;
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

public class ListPolicyProfilesTag extends StyleTagSupport {
    private DeviceHome m_deviceHome = null;
    private DeviceGroupHome m_deviceGroupHome = null;
    private UserHome m_userHome = null;
    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            // OPtimization to look up all home interfaces here
            if ( m_deviceHome == null ) {
                m_deviceHome = ( DeviceHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  DeviceHome.class,
                                                                    "Device" );

                m_userHome = ( UserHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  UserHome.class,
                                                                    "User" );

                m_deviceGroupHome = ( DeviceGroupHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  DeviceGroupHome.class,
                                                                    "DeviceGroup" );
            }

            Element documentRootElement = new Element("items");
            // Find the unordered collection of EJB objects
            Collection items = m_deviceHome.findByOrganizationID( new Integer ( getOrganizationID() ) );
            for (Iterator iter = items.iterator(); iter.hasNext(); ) {
                Object nextEJBObject = iter.next();
                Element nextElement =
                    createElementFromObject("item", nextEJBObject);
                // Expand on any SubElements in the EJB Object
                // displaying friendly names etc.
                Element fullyParsedElement = postProcessElement ( nextElement );
                documentRootElement.addContent( fullyParsedElement );
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
      * Post Process the XML and insert friendly sub elements where
      * we have primmary keys
      */
    protected Element postProcessElement( Element inputElement )
        throws PostProcessingException {
        try {
            Element userIDChildElement = inputElement.getChild( "userid" );
            String userID = userIDChildElement.getText();

            User user = m_userHome.findByPrimaryKey( userID );

            Element displayidElement = new Element("displayid");
            displayidElement.setText(user.getDisplayID());
            userIDChildElement.addContent(displayidElement);

            Element deviceGroupIDChildElement = inputElement.getChild( "phonegroupid" );
            Integer deviceGroupID = new Integer (deviceGroupIDChildElement.getText());
            DeviceGroup deviceGroup = m_deviceGroupHome.findByPrimaryKey( deviceGroupID );

            Element nameElement = new Element("name");
            nameElement.setText(deviceGroup.getName());
            deviceGroupIDChildElement.addContent(nameElement);

            return inputElement;
        } catch ( Exception ex ) {
            throw new PostProcessingException(ex.getMessage());
        }
    }

    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        super.clearProperties();
    }
}
