/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/ListSoftwareTag.java#5 $
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
import com.pingtel.pds.pgs.phone.CoreSoftwareHome;
import com.pingtel.pds.pgs.phone.DeviceType;
import com.pingtel.pds.pgs.phone.DeviceTypeHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.Collection;
import java.util.Iterator;

/**
 * <p>Title: ListSoftwareTag</p>
 * <p>Description: Tag Library for the DMS JSP Pages</p>
 * <p>Copyright: Copyright (c) 2002</p>
 * <p>Company: Pingtel Corp</p>
 * @author John P. Coffey
 * @version 1.0
 */

public class ListSoftwareTag extends StyleTagSupport {
    private CoreSoftwareHome m_coreSoftwareHome = null;
    private DeviceTypeHome m_deviceTypeHome = null;
    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            // OPtimization to look up all home interfaces here
            if ( m_coreSoftwareHome == null ) {
                m_coreSoftwareHome = ( CoreSoftwareHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  CoreSoftwareHome.class,
                                                                    "CoreSoftware" );
            }

            Element documentRootElement = new Element("items");
            // Start by finding the all the DeviceTypes in the system
            Collection deviceTypes = m_deviceTypeHome.findAll();
            for (Iterator dtIter = deviceTypes.iterator(); dtIter.hasNext(); ) {
                DeviceType deviceType  = (DeviceType) dtIter.next();

                // Now use the custom finder on the core software details
                Collection coreSWItems =
                    m_coreSoftwareHome.
                        findByDeviceType( deviceType.getID() );

                // iterate over all the coreSWItems for a particular deviceType
                for (Iterator csIter = coreSWItems.iterator(); csIter.hasNext(); ) {
                    Object nextEJBObject = csIter.next();
                    Element nextElement = createElementFromObject("item", nextEJBObject);

                    // Expand on any SubElements in the EJB Object
                    // displaying friendly names etc.
                    Element fullyParsedElement = postProcessElement ( nextElement );

                    // Now in this case postProcessing the element does not
                    // add the deviceType name to the element as it comes
                    // from a joines bean, so....
                    fullyParsedElement.addContent(
                        new Element ("assoc_devicetype_model").
                            setText(deviceType.getModel()));

                    documentRootElement.addContent( fullyParsedElement );
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
      * Post Process the XML and insert friendly sub elements where
      * we have primmary keys
      */
    protected Element postProcessElement( Element inputElement )
        throws PostProcessingException {
            return inputElement;
    }

    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        super.clearProperties();
    }
}
