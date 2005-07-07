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

import com.pingtel.pds.common.MD5Encoder;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.user.UserHome;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserAdvocateHome;
import com.pingtel.pds.pgs.user.UserAdvocate;
import com.pingtel.pds.pgs.organization.OrganizationHome;
import org.apache.log4j.Category;

import javax.naming.InitialContext;
import javax.rmi.PortableRemoteObject;
import java.util.HashMap;
import java.util.Collection;
import java.util.Iterator;


public class SDSPasswordRedigester implements Patch {

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

            objref = initial.lookup( "UserAdvocate" );
            UserAdvocateHome userAdvocateHome =
                    ( UserAdvocateHome ) PortableRemoteObject.
                        narrow( objref, UserAdvocateHome.class );

            UserAdvocate userAdvocate = userAdvocateHome.create();

            User sds = null;
            Collection c = userHome.findByDisplayID( "SDS" );
            for ( Iterator i = c.iterator(); i.hasNext(); )
                sds = (User) i.next();

            userAdvocate.setUsersPassword( sds.getID().toString(), "SDS", "SDS" );
        }
        catch ( Exception ex ) {
            m_logger.error ( "Exception in PasswordDigestor: " + ex.toString() );

            throw new PDSException ( ex.toString() );
        }

        return "S";
    }
}
