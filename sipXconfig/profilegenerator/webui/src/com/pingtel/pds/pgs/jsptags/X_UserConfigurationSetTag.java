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
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.profile.ConfigurationSet;
import com.pingtel.pds.pgs.profile.ConfigurationSetHome;
import com.pingtel.pds.pgs.profile.RefProperty;
import com.pingtel.pds.pgs.profile.RefPropertyHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.StringTokenizer;


public class X_UserConfigurationSetTag extends StyleTagSupport {

    private String m_userID;
    public int m_profileType = -1;
    private ArrayList m_refPropertyCodes;

    private RefPropertyHome m_rpHome = null;
    private ConfigurationSetHome m_csHome = null;

    public void setUserid ( String userid ) {
        this.m_userID = userid;
    }

    public void setProfiletype ( String profiletype ) {
        this.m_profileType = new Integer ( profiletype ).intValue();
    }

    public void setRefpropertycodes ( String refpropertycodes ) {
        m_refPropertyCodes = new ArrayList();
        StringTokenizer tok = new StringTokenizer ( refpropertycodes, "," );

        while ( tok.hasMoreTokens() ) {
            String token = tok.nextToken();
            m_refPropertyCodes.add ( token );
        }
    }


    public int doStartTag() throws JspException {
        try {
            if (  m_profileType != PDSDefinitions.PROF_TYPE_USER ) {
                throw new JspException ( "invalid profile type for" );
            }

            // Optimization to look up all home interfaces here
            if ( m_rpHome == null ) {
                m_rpHome = ( RefPropertyHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  RefPropertyHome.class,
                                                                    "RefProperty" );

                m_csHome = ( ConfigurationSetHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ConfigurationSetHome.class,
                                                                    "ConfigurationSet" );
            }

            Element documentRootElement = new Element("userdetails");

            Element csElement = new Element ( "configurationset" );
            documentRootElement.addContent( csElement );

            Collection cCS =
                m_csHome.findByUserIDAndProfileType( m_userID, m_profileType );

            for ( Iterator iCS = cCS.iterator(); iCS.hasNext(); ) {
                ConfigurationSet cs = (ConfigurationSet) iCS.next();
                csElement.addContent( cs.getContent() );
            }

            Element refPropertiesElement = new Element ( "refproperties" );
            documentRootElement.addContent( refPropertiesElement );

            for ( Iterator iRP = m_refPropertyCodes.iterator(); iRP.hasNext(); ) {

                String rpCode = (String) iRP.next();
                RefProperty rp = null;
                Collection rpC = m_rpHome.findByCode( rpCode );
                for ( Iterator iC = rpC.iterator(); iC.hasNext(); )
                    rp = (RefProperty) iC.next();

                if ( rp == null )
                    throw new JspException ( "No ref property exist with that code" );

                Element rpElement = new Element ( "refproperty" );
                refPropertiesElement.addContent( rpElement );
                
                Element code = new Element ( "code");
                code.addContent( rp.getCode() );
                rpElement.addContent( code  );
                Element id = new Element ( "id" );
                id.addContent( rp.getID().toString());
                rpElement.addContent( id );
                Element name = new Element ( "name" );
                name.addContent( rp.getName());
                rpElement.addContent( name );
                Element content = new Element ( "content" );
                content.addContent(  rp.getContent() );
                rpElement.addContent( content );
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
        m_userID = null;
        m_profileType = -1;
        m_refPropertyCodes = null;

        super.clearProperties();
    }

}