/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/GetRefConfigSetsTag.java#4 $
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
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.profile.RefConfigurationSet;
import com.pingtel.pds.pgs.profile.RefConfigurationSetHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;
import java.util.*;

public class GetRefConfigSetsTag extends ExTagSupport {

    private String orgID;
    private RefConfigurationSetHome refConfHome = null;

    public void setOrgid( String orgid ) {
    this.orgID = orgid;
    }

    public int doStartTag() throws JspException {
        try {
            if ( refConfHome == null ) {
                refConfHome = ( RefConfigurationSetHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  RefConfigurationSetHome.class,
                                                                    "RefConfigurationSet" );
            }

            // use a tree map for sorting the collection
            TreeMap map1 =new TreeMap();

            // get a collection of refconfigsets
            Collection refConfSetCollection = ( Collection )
                                           refConfHome.findByOrganizationID( new Integer( orgID ) );

            RefConfigurationSet refSet = null;
            for ( Iterator i = refConfSetCollection.iterator(); i.hasNext(); )
            {
                refSet = ( RefConfigurationSet ) i.next();
                map1.put( refSet.getName(), refSet.getID() );
            }

            // place sorted set in a selection input box
            StringBuffer sBuffer = new StringBuffer();
            Set set1 = map1.entrySet();
            Iterator map1Iterator = set1.iterator();
            while ( map1Iterator.hasNext() )
            {
                Map.Entry me = ( Map.Entry ) map1Iterator.next();
                sBuffer.append("<option value=\"" +
                               me.getValue() + "\">" +
                               me.getKey() +
                               "\n");
            }

            // output to browser
            pageContext.getOut().println( "<select name=\"refconfigsetid\" size=\"4\">" );
            pageContext.getOut().println( sBuffer );
            pageContext.getOut().println( "</select>" );
        }
        catch(Exception ioe ){
            throw new JspTagException( ioe.getMessage() );
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        orgID = null;

        super.clearProperties();
    }
}