/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/ListDevicesGroupsFilterTag.java#5 $
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
import com.pingtel.pds.common.ElementUtilException;
import com.pingtel.pds.common.PostProcessingException;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.phone.DeviceGroup;
import com.pingtel.pds.pgs.phone.DeviceGroupHome;
import org.jdom.Element;

import javax.ejb.FinderException;
import javax.servlet.jsp.JspException;
import java.rmi.RemoteException;
import java.util.Collection;
import java.util.Iterator;


public class ListDevicesGroupsFilterTag extends StyleTagSupport {

    private DeviceGroupHome m_deviceGroupHome = null;

    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            if ( m_deviceGroupHome == null ) {
                m_deviceGroupHome = (DeviceGroupHome)
                    EJBHomeFactory.getInstance().getHomeInterface( DeviceGroupHome.class, "DeviceGroup" );
            }

            Element root = new Element ( "groups" );

            Collection all = m_deviceGroupHome.findAll();
            for ( Iterator iGroup = all.iterator(); iGroup.hasNext(); ) {
                DeviceGroup dg = (DeviceGroup) iGroup.next();
                if ( dg.getParentID() == null ) {
                    buildGroups ( dg, root );
                }
            }

            outputTextToBrowser ( root );
        }
        catch(Exception ex ) {
             throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }


    private Element buildGroups ( DeviceGroup deviceGroup, Element node )
        throws  RemoteException,
                FinderException,
                ElementUtilException,
                PostProcessingException {

        Element groupElement = new Element ( "group" );
        groupElement.addContent ( new Element ( "name" ).setText( deviceGroup.getName() ) );
        groupElement.addContent( new Element ( "id" ).setText( deviceGroup.getID().toString( ) ) );

        node.addContent( groupElement );

        Collection childGroups =
            m_deviceGroupHome.findByParentID( deviceGroup.getID() );

        for ( Iterator iGroup = childGroups.iterator(); iGroup.hasNext(); ) {
            DeviceGroup dg = (DeviceGroup) iGroup.next();
            buildGroups (  dg, groupElement);
        }

        return node;
    }



    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        super.clearProperties();
    }
}
