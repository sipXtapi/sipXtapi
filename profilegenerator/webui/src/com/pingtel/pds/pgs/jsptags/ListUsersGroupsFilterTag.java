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
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.ElementUtilException;
import com.pingtel.pds.common.PostProcessingException;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.user.UserGroup;
import com.pingtel.pds.pgs.user.UserGroupHome;
import org.jdom.Element;

import javax.ejb.FinderException;
import javax.servlet.jsp.JspException;
import java.rmi.RemoteException;
import java.util.Collection;
import java.util.Iterator;



public class ListUsersGroupsFilterTag extends StyleTagSupport {

    private UserGroupHome m_userGroupHome = null;


    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            if ( m_userGroupHome == null ) {
                m_userGroupHome = ( UserGroupHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  UserGroupHome.class,
                                                                    "UserGroup" );
            }

            Element root = new Element ( "groups" );

            Collection all = m_userGroupHome.findAll();
            for ( Iterator iGroup = all.iterator(); iGroup.hasNext(); ) {
                UserGroup ug = (UserGroup) iGroup.next();
                if ( ug.getParentID() == null ) {
                    buildGroups ( ug, root );
                }
            }

            outputTextToBrowser ( root );

        }
        catch(Exception ex ) {
             throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }


    private Element buildGroups ( UserGroup userGroup, Element node )
        throws  RemoteException,
                FinderException,
                ElementUtilException,
                PostProcessingException,
                PDSException {


        Element groupElement = new Element ( "group" );

        groupElement.addContent ( new Element ( "name" ).setText( userGroup.getName() ) );
        groupElement.addContent( new Element ( "id" ).setText( userGroup.getID().toString( ) ) );

        node.addContent( groupElement );

        Collection childGroups =
            m_userGroupHome.findByParentID( userGroup.getID() );

        for ( Iterator iGroup = childGroups.iterator(); iGroup.hasNext(); ) {
            UserGroup ug = (UserGroup) iGroup.next();
            buildGroups (  ug, groupElement);
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
