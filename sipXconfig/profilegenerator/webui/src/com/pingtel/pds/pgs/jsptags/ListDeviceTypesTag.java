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
import com.pingtel.pds.pgs.phone.DeviceTypeHome;
import com.pingtel.pds.pgs.phone.Manufacturer;
import com.pingtel.pds.pgs.phone.ManufacturerHome;
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

public class ListDeviceTypesTag extends StyleTagSupport {
    private DeviceTypeHome m_deviceTypeHome = null;
    private ManufacturerHome m_manufacturerHome = null;
    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            // OPtimization to look up all home interfaces here
            if ( m_deviceTypeHome == null ) {
                m_deviceTypeHome = ( DeviceTypeHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  DeviceTypeHome.class,
                                                                    "DeviceType" );

                m_manufacturerHome = ( ManufacturerHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ManufacturerHome.class,
                                                                    "Manufacturer" );
            }

            Element documentRootElement = new Element("items");
            // Find the unordered collection of EJB objects
            // note that this is not specific to a particular organization
            Collection items = m_deviceTypeHome.findAll();
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
            String manufacturerID =
                inputElement.
                    getChild( "manufacturerid" ).getText();

            if ( manufacturerID != "" ) {
                Manufacturer manufacturer =
                    m_manufacturerHome.
                        findByPrimaryKey(
                            new Integer (manufacturerID));

                // Only add details if the id exists
                if (manufacturer != null) {
                    inputElement.addContent(
                        new Element ("manufacturerid_name").
                            setText(manufacturer.getName()));
                }
            }
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
