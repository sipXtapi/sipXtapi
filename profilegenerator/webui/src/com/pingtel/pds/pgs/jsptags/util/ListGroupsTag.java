/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/util/ListGroupsTag.java#5 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.jsptags.util;

import java.io.IOException;
import java.rmi.RemoteException;
import java.util.Collection;

import javax.ejb.FinderException;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.rmi.PortableRemoteObject;
import javax.servlet.jsp.JspException;

import org.jdom.Element;

import com.pingtel.pds.common.ElementUtilException;
import com.pingtel.pds.common.PostProcessingException;
import com.pingtel.pds.pgs.phone.DeviceGroupHome;
import com.pingtel.pds.pgs.user.UserGroupHome;

/**
 * <p>Title: DMSTags</p>
 * <p>Description: Tag Library for the DMS JSP Pages</p>
 * <p>Copyright: Copyright (c) 2002</p>
 * <p>Company: Pingtel Corp</p>
 * @author John P. Coffey
 * @version 1.0
 */

public class ListGroupsTag extends StyleTagSupport {
    private String m_type;

    /** Sets teh type of the group here */
    public void setType( String type ) {
        m_type = type;
    }

    /**
     * This is where we look up the UserGroup
     */
    public int doStartTag() throws JspException {
        try {
            // Perform the normal JNDI Lookup Code
            InitialContext initial = new InitialContext();
            // UpperCase the first character of the type e.g. UserGroup, DeviceGroup
            String jndiLookupName =
                jndiLookupName = m_type.substring(0, 1).toUpperCase() +
                                 m_type.substring(1) + "Group";

            Object objref = initial.lookup( jndiLookupName );

            // Call superclass helper method to get the org id
            String orgID = getOrganizationID();

            Collection unorderedGroupObjects = null;
            if (jndiLookupName.equals("UserGroup")) {
                // Use a custom finder to retrieve all of the organization's
                // user groups, (note that these are proxy objects)
                UserGroupHome home = ( UserGroupHome )
                    PortableRemoteObject.narrow( objref, UserGroupHome.class );

                // Retrieve a databasse unordered collection of UserGroup EJB objects
                unorderedGroupObjects = home.findByOrganizationID( new Integer (orgID) );
            } else if (jndiLookupName.equals("DeviceGroup")) {
                // Use a custom finder to retrieve all of the organization's
                // user groups, (note that these are proxy objects)
                DeviceGroupHome home = ( DeviceGroupHome )
                    PortableRemoteObject.narrow( objref, DeviceGroupHome.class );

                // Retrieve a databasse unordered collection of UserGroup EJB objects
                unorderedGroupObjects = home.findByOrganizationID( new Integer (orgID) );
            } else {
                 throw new JspException(
                    "Illegal type parameter passed to tag library: " + m_type );
            }

            // Call the utility helper class to sort and order these
            Element documentRootNode =
                ElementUtil.getInstance().
                    sortCollection("groups", "group", unorderedGroupObjects );
            // Sends the text to the browser in either xml or html format
            // depending on an optional debug tag attribute
            outputTextToBrowser ( documentRootNode );
        } catch( ElementUtilException ex ) {
             throw new JspException(
                "ElementUtilException: " + ex.getMessage());
        } catch( FinderException ex ) {
             throw new JspException(
                "FinderException: " + ex.getMessage());
        } catch( NamingException ex ) {
             throw new JspException(
                "NamingException: " + ex.getMessage());
        } catch( RemoteException ex ) {
            throw new JspException(
                "RemoteException: " + ex.getMessage());
        } catch( IOException ex ){
             throw new JspException(
                "IOException: " + ex.getMessage());
        }
        return SKIP_BODY;
    }

    /** Right now the ListGroupsTag does not need to post process an element */
    protected Element postProcessElement(Element element)
        throws PostProcessingException {
        // this is never called here
        return element;
    }

    /**
     * Just reset the type here
     */
    protected void clearProperties() {
        m_type = null;
        super.clearProperties();
    }
}
