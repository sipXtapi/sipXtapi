/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/patch/InitialLineMigrator.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.patch;

import javax.naming.*;
import javax.ejb.*;
import com.pingtel.pds.pgs.user.*;
import com.pingtel.pds.pgs.profile.*;
import com.pingtel.pds.common.PDSException;
import java.util.*;
import java.io.*;
import javax.rmi.PortableRemoteObject;
import org.jdom.*;
import org.jdom.input.*;
import org.jdom.output.*;
import org.apache.log4j.*;

public class InitialLineMigrator implements Patch {

    private XMLOutputter m_xmlOut = new XMLOutputter();
    private Category m_logger = Category.getInstance( "pgs" );

    public void initialize(HashMap props) {}

    public String update() throws PDSException {

        UserHome userHome = null;
        ConfigurationSetHome csHome = null;
        RefPropertyHome rpHome = null;

        SAXBuilder saxBuilder = new SAXBuilder();

        try {
            //InitialContext initial = new InitialContext();
            InitialContext initial = new InitialContext();

            Object objref = initial.lookup( "User" );
                userHome =
                    ( UserHome ) PortableRemoteObject.
                        narrow( objref, UserHome.class );

            objref = initial.lookup( "ConfigurationSet" );
                csHome =
                    ( ConfigurationSetHome ) PortableRemoteObject.
                        narrow( objref, ConfigurationSetHome.class );

            objref = initial.lookup( "RefProperty" );
                rpHome =
                    ( RefPropertyHome ) PortableRemoteObject.
                        narrow( objref, RefPropertyHome.class );


            RefProperty userLine = null;
            RefProperty primaryLine = null;

            Collection ulC = rpHome.findByCode( "xp_2029" );
            for ( Iterator iULC = ulC.iterator(); iULC.hasNext(); )
                userLine = (RefProperty) iULC.next();

            Collection plC = rpHome.findByCode( "xp_10001" );
            for ( Iterator iPLC = plC.iterator(); iPLC.hasNext(); )
                primaryLine = (RefProperty) iPLC.next();


            Collection allUsers = userHome.findAll();
            for ( Iterator iUser = allUsers.iterator(); iUser.hasNext(); ) {
                User user = (User) iUser.next();
                String plURL = user.calculatePrimaryLineURL();
                Collection configSets = csHome.findByUserID( user.getID() );
                for ( Iterator iCS = configSets.iterator(); iCS.hasNext(); ) {
                    ConfigurationSet cs = (ConfigurationSet) iCS.next();
                    String content = cs.getContent();
                    Document doc =
                        saxBuilder.build( new ByteArrayInputStream ( content.getBytes() ) );

                    Element root = doc.getRootElement();

                    String newContent =
                        fixPrimaryLine (    root,
                                            userLine.getID().toString(),
                                            primaryLine.getID().toString(),
                                            plURL );

                    cs.setContent( newContent );
                }
            }

        }
        catch ( Exception ex ) {
            m_logger.error ( "Exception in InitialLineMigrator: " + ex.toString() );
            throw new PDSException ( ex.toString() );
        }

        return "S";
    }

    private String fixPrimaryLine ( Element content, String userLineID, String primaryLineID, String plURL )
        throws Exception {

        Collection settings = content.getChildren();
        for ( Iterator iSetting = settings.iterator(); iSetting.hasNext(); ) {
            Element setting = (Element) iSetting.next();
            if ( setting.getAttributeValue( "ref_property_id" ).equals( userLineID ) ) {
                Element wrapper = setting.getChild( "USER_LINE" );
                Element url = wrapper.getChild( "URL" );
                if ( url.getText().equals( plURL ) ) {
                    setting.setAttribute( "ref_property_id", primaryLineID );
                    setting.removeAttribute( "id" );
                    setting.setName( "PRIMARY_LINE" );
                    wrapper.setName( "PRIMARY_LINE" );
                }
            }
        }

        return m_xmlOut.outputString( content );
    }

}