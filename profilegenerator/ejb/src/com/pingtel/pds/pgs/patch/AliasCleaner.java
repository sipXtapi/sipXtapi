/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/patch/AliasCleaner.java#4 $
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

import javax.naming.InitialContext;
import javax.rmi.PortableRemoteObject;

import java.util.HashMap;
import java.util.Collection;
import java.util.Iterator;
import java.util.StringTokenizer;

import org.apache.log4j.Category;

import com.pingtel.pds.common.MD5Encoder;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.user.UserHome;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.organization.OrganizationHome;


public class AliasCleaner implements Patch {

    private MD5Encoder m_md5Encoder = new MD5Encoder();
    private Category m_logger = Category.getInstance( "pgs" );

    public void initialize(HashMap props) {}

    public String update() throws PDSException {

        UserHome userHome = null;

        try {
            InitialContext initial = new InitialContext();

            Object objref = initial.lookup( "User" );
                userHome =
                    ( UserHome ) PortableRemoteObject.
                        narrow( objref, UserHome.class );


            Collection allUsers = userHome.findAll();
            for ( Iterator iUser = allUsers.iterator(); iUser.hasNext(); ) {
                User user = (User) iUser.next();
                m_logger.debug( "examining user: " + user.getExternalID() );

                StringBuffer cleanedAliases = new StringBuffer();

                String existingAliases = user.getAliases();
                StringTokenizer aliasTok = new StringTokenizer ( existingAliases, "," );
                while ( aliasTok.hasMoreTokens() ) {
                    String oldAlias = aliasTok.nextToken();

                    if ( oldAlias.toLowerCase().startsWith( "sip:" )) {
                        oldAlias = oldAlias.substring( 4 );
                    }

                    if ( oldAlias.indexOf( '@') != -1 ) {
                        oldAlias = oldAlias.substring( 0, oldAlias.indexOf( '@') );
                    }

                    cleanedAliases.append( oldAlias );
                    if ( aliasTok.hasMoreTokens() ) {
                        cleanedAliases.append( "," );
                    }
                }

                user.setAliases ( cleanedAliases.toString() );
            }

        }
        catch ( Exception ex ) {
            m_logger.error ( "Exception in PasswordDigestor: " + ex.toString() );

            throw new PDSException ( ex.toString() );
        }

        return "S";
    }

}