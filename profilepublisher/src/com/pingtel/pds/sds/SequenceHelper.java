/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/com/pingtel/pds/sds/SequenceHelper.java#6 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.sds;

import javax.naming.InitialContext;
import javax.rmi.PortableRemoteObject;
import javax.security.auth.login.LoginContext;
import javax.security.auth.login.LoginException;

import org.apache.log4j.Category;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.phone.DeviceAdvocate;
import com.pingtel.pds.pgs.phone.DeviceAdvocateHome;
import org.jboss.security.auth.callback.UsernamePasswordHandler;

/**
 * Title:
 * Description:  Abstracts the finding of sequence numbers from the Mainline Code
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class SequenceHelper {

    private static Category m_logger;

    static {
        m_logger = Category.getInstance( "pds" );
    }

    /** Singleton */
    private static SequenceHelper g_instance = null;
    /** Store Store the PhoneTagAdvocate home interface here */
    private ThreadLocal m_threadLocalDeviceAdvocateHome = new ThreadLocal();
    /** JAAS userid (this is a  hack that needs to be removed) */
    private String m_jaasuserid = "SDS";
    /** JAAS password (this is a  hack that needs to be removed) */
    private String m_jaaspassword = "SDS";
    /** Use this to authenticate with EJB */
    private LoginContext m_LoginContext;
    /** @todo remove this duplicate in favor of jdbc search */
    private static final String PROPERTIES_FILE = "sds.props";

    /** Private constructor for singletons */
    private SequenceHelper() {
    }

    /** Singleton Accessor for the singleton */
    public static SequenceHelper getInstance() {
        if (g_instance == null) {
            g_instance = new SequenceHelper();
        }
        return g_instance;
    }

    /** Finds the sequence number for the enumerated type and MacAddress */
    public int getProfileSequenceNumber ( int profileType, String macAddress )
        throws PDSException {

        m_logger.debug( "fetching profile sequence number for " + macAddress );
        // Attempt to use the EJB JNDI mechanism first
        int sequenceNumber = -1;
        try {
            // Multi Threaded Support for the Initial Context
            // this needs to be initialized on a per-thread basis
            // so we store it in ThreadLocal storage object
            DeviceAdvocateHome deviceAdvocateHome =
                (DeviceAdvocateHome)m_threadLocalDeviceAdvocateHome.get();

            m_logger.debug( "got DeviceAdvocateHome interface" );

            if ( deviceAdvocateHome == null ) {
                // Need to query the Thread specific phone tag advocate
                // home from the Initial Context and store it in thread
                // safe storage

                // Firstly authenticate via JAAS
                authenticate (m_jaasuserid, m_jaaspassword);

                m_logger.debug ( "authenticated with PGS" );

                // Create the initial context, this is the tricky bit
                // as to call this method reliably requres the Application
                // class loader rather than the null one that accompanies
                // the C++ thread context (within which this method is called)
                InitialContext context = new InitialContext();
                m_logger.debug ( "got InitialContext" );
                Object objRef = context.lookup("DeviceAdvocate");
                deviceAdvocateHome =
                    (DeviceAdvocateHome) PortableRemoteObject.
                        narrow( objRef, DeviceAdvocateHome.class );

                m_logger.debug( "got DeviceAdvocate interface" );
                m_threadLocalDeviceAdvocateHome.set( deviceAdvocateHome );
            }

            // Find the PhoneTagAdvocate, Stateless Session Bean
            DeviceAdvocate deviceAdvocate = deviceAdvocateHome.create();
            sequenceNumber = deviceAdvocate.fetchSequenceNumber( macAddress, profileType );
            m_logger.debug( "fetch sequence# " + sequenceNumber + " for serial# " +
                            macAddress );
        } catch (Exception ex) {
            throw new PDSException(ex.getMessage());
        }
        return sequenceNumber;
    }

    /**
     * JAAS Authentication & authroization (this is a security risk as the
     * userid and passwords are stored with this app
     */
    private void authenticate( String principal, String password ) throws LoginException {
        char[] pw = password.toCharArray();
        UsernamePasswordHandler handler = new UsernamePasswordHandler(principal, pw);
        m_LoginContext = new LoginContext("other", handler);
        m_LoginContext.login();
    }
}