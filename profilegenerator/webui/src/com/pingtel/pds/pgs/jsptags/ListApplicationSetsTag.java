/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/ListApplicationSetsTag.java#5 $
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
import com.pingtel.pds.pgs.user.ApplicationGroup;
import com.pingtel.pds.pgs.user.ApplicationGroupHome;
import org.jdom.Element;
import org.apache.regexp.RE;
import org.apache.regexp.RESyntaxException;

import javax.servlet.jsp.JspException;
import java.util.Collection;
import java.util.Iterator;


public class ListApplicationSetsTag extends StyleTagSupport {
    private ApplicationGroupHome m_agHome = null;

    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            // OPtimization to look up all home interfaces here
            if ( m_agHome == null ) {
                m_agHome = ( ApplicationGroupHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationGroupHome.class,
                                                                    "ApplicationGroup" );
            }

            Element documentRootElement = new Element("items");

            // Find the unordered collection of System Administrators
            Collection items = m_agHome.
                findByOrganizationID( new Integer (getOrganizationID()) );

            // Add each one of these to the document root element and post process it
            for (Iterator iter = items.iterator(); iter.hasNext(); ) {
                //Object nextEJBObject = iter.next();
                ApplicationGroup nextEJBObject = (ApplicationGroup) iter.next();
                String name = nextEJBObject.getName();

                if ( ! isSingleAppSet (name) ) {
                    Element nextElement =
                        createElementFromObject("item", nextEJBObject);
                    // Expand on any SubElements in the EJB Object
                    // displaying friendly names etc.
                    Element fullyParsedElement = postProcessElement ( nextElement );
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


    ///////////////////////////////////////////////////////////////////////
    //
    // isSingleAppSet determines whether an application set is used just to
    // assign a single application specifically to a user/user group.   We
    // don't want to show those in the UI
    //
    ///////////////////////////////////////////////////////////////////////
    private boolean isSingleAppSet ( String appSetName ) {
        boolean returnValue = false;

        try {
            RE hiddernAppSetsRE = new RE ("^(u|ug)[0-9]+_");
            returnValue = hiddernAppSetsRE.match(appSetName);
        }
        catch (RESyntaxException e) {
            e.printStackTrace();
        }

        return returnValue;
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
