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
import com.pingtel.pds.pgs.user.ApplicationGroup;
import com.pingtel.pds.pgs.user.ApplicationGroupHome;
import com.pingtel.pds.pgs.user.ApplicationHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;


public class ListUsersAvailApplicationSetsTag extends StyleTagSupport {

    private ApplicationHome m_applicationHome = null;
    private ApplicationGroupHome m_agHome = null;

    private String m_userID;

    public void setUserid ( String userid ) {
        this.m_userID = userid;
    }

    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            // OPtimization to look up all home interfaces here
            if ( m_applicationHome == null ) {
                m_applicationHome = ( ApplicationHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationHome.class,
                                                                    "Application" );

                m_agHome = ( ApplicationGroupHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationGroupHome.class,
                                                                    "ApplicationGroup" );
            }

            Collection allAG = m_agHome.findAll();
            ArrayList privateAGs = new ArrayList();

            for ( Iterator iAll = allAG.iterator(); iAll.hasNext(); ) {
                ApplicationGroup ag = (ApplicationGroup) iAll.next();
                if ( isPrivateAppSet( ag.getName() ) ) {

                    if ( allAG.contains( ag ) ) {
                        privateAGs.add ( ag );
                    }
                }
            }

            allAG.removeAll( privateAGs );

            Element documentRootElement = new Element("items");

            Collection existing = m_agHome.findByUserID( m_userID );

            for ( Iterator iUser = existing.iterator(); iUser.hasNext(); ) {
                ApplicationGroup ag = (ApplicationGroup) iUser.next();

                if ( allAG.contains( ag ) ) {
                    allAG.remove( ag );
                }
            }

            for ( Iterator filteredI = allAG.iterator(); filteredI.hasNext(); ) {
                ApplicationGroup ag = (ApplicationGroup) filteredI.next();

                Element appElement = createElementFromObject( "applicationset", ag );
                documentRootElement.addContent( appElement );
             }


            outputTextToBrowser ( documentRootElement );
        }
        catch(Exception ex ) {
             throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }


    private boolean isPrivateAppSet ( String name ) {
        boolean returnValue = false;

        if ( name.startsWith( "u" ) || name.startsWith( "ug" ) ) {
            String trimmed = null;

            if ( name.startsWith( "ug" ) ) {
                trimmed = name.substring( 2 );
            }
            else {
                trimmed = name.substring( 1 );
            }
            char [] chars = trimmed.toCharArray();
            for ( int i = 0; i < chars.length; ++i ) {
                switch ( chars [ i ] ) {
                    case '_' :{
                        break;
                    }
                    case '0' :{
                        returnValue = true;
                        break;
                    }
                    case '1' :{
                        returnValue = true;
                        break;
                    }
                    case '2' :{
                        returnValue = true;
                        break;
                    }
                    case '3' :{
                        returnValue = true;
                        break;
                    }
                    case '4' :{
                        returnValue = true;
                        break;
                    }
                    case '5' :{
                        returnValue = true;
                        break;
                    }
                    case '6' :{
                        returnValue = true;
                        break;
                    }
                    case '7' :{
                        returnValue = true;
                        break;
                    }
                    case '8' :{
                        returnValue = true;
                        break;
                    }
                    case '9' :{
                        returnValue = true;
                        break;
                    }
                    default :  {
                        returnValue = false;
                        break;
                    }
                } // switch
            } // for
        } // if

        return returnValue;
    }

    /**
      * Post Process the XML and insert friendly sub elements where
      * we have primmary keys
      */
    protected Element postProcessElement( Element inputElement )
        throws PostProcessingException {
        // @JC TODO
        return inputElement;
    }

    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        m_userID = null;

        super.clearProperties();
    }
}
