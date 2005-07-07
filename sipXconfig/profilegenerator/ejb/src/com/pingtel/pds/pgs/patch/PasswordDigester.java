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

package com.pingtel.pds.pgs.patch;

import javax.naming.*;
import javax.ejb.*;
import com.pingtel.pds.pgs.user.*;
import com.pingtel.pds.pgs.organization.*;
import com.pingtel.pds.pgs.profile.*;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.MD5Encoder;
import java.util.*;
import java.io.*;
import javax.rmi.PortableRemoteObject;
import org.jdom.*;
import org.jdom.input.*;
import org.jdom.output.*;
import org.apache.log4j.*;

public class PasswordDigester implements Patch {

    private MD5Encoder m_md5Encoder = new MD5Encoder();
    private Category m_logger = Category.getInstance( "pgs" );

    public void initialize(HashMap props) {}

    public String update() throws PDSException {

        UserHome userHome = null;
        OrganizationHome organizationHome = null;

        try {
            InitialContext initial = new InitialContext();

            Object objref = initial.lookup( "User" );
                userHome =
                    ( UserHome ) PortableRemoteObject.
                        narrow( objref, UserHome.class );

            objref = initial.lookup( "Organization" );
                organizationHome =
                    ( OrganizationHome ) PortableRemoteObject.
                        narrow( objref, OrganizationHome.class );


            Collection allUsers = userHome.findAll();
            for ( Iterator iUser = allUsers.iterator(); iUser.hasNext(); ) {
                User user = (User) iUser.next();
                m_logger.debug( "examining user: " + user.getExternalID() );

                String existingPassword = user.getPassword();
                if ( existingPassword == null || existingPassword.length() != 32 ) {

                    if ( existingPassword != null ) {
                        user.setPassword( existingPassword );
                    }
                    else {
                        user.setPassword( "" );
                    }

                    m_logger.debug( "set password for user to: " + user.getPassword() );
                }
            }

        }
        catch ( Exception ex ) {
            m_logger.error ( "Exception in PasswordDigestor: " + ex.toString() );

            throw new PDSException ( ex.toString() );
        }

        return "S";
    }

}