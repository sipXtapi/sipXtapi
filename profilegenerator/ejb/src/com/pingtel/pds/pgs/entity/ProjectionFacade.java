/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/entity/ProjectionFacade.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
package com.pingtel.pds.pgs.entity;

import javax.ejb.EJBException;
import javax.ejb.MessageDrivenBean;
import javax.ejb.MessageDrivenContext;
import javax.jms.JMSException;
import javax.jms.MapMessage;
import javax.jms.Message;
import javax.jms.MessageListener;
import javax.naming.InitialContext;

import java.rmi.RemoteException;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.ejb.BaseEJB;
import com.pingtel.pds.pgs.phone.DeviceGroupAdvocate;
import com.pingtel.pds.pgs.phone.DeviceGroupAdvocateHome;
import com.pingtel.pds.pgs.user.UserGroupAdvocate;
import com.pingtel.pds.pgs.user.UserGroupAdvocateHome;


/**
 * ProjectionFacade is the MessageDrivenBean implementation class.
 * ProjectionFacade is an aysnchronous session facade for doing
 * group projections.
 * @author IB
 * @see #onMessage
 */
public class ProjectionFacade extends BaseEJB
        implements MessageDrivenBean, MessageListener {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    private MessageDrivenContext mCTX = null;
    private UserGroupAdvocate mUserGroupAdvocate = null;
    private DeviceGroupAdvocate mDeviceGroupAdvocate = null;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////
    public ProjectionFacade() { }

    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    /**
     *  Standard EJB implementation
     */
    public void setMessageDrivenContext( MessageDrivenContext ctx )
            throws EJBException {

        this.mCTX = ctx;

        try {
            InitialContext initial = new InitialContext( );

            UserGroupAdvocateHome userGroupAdvocateHome =
                        (UserGroupAdvocateHome) initial.lookup( "UserGroupAdvocate");

            mUserGroupAdvocate = userGroupAdvocateHome.create();

            DeviceGroupAdvocateHome deviceGroupAdvocateHome =
                        (DeviceGroupAdvocateHome) initial.lookup( "DeviceGroupAdvocate");

            mDeviceGroupAdvocate = deviceGroupAdvocateHome.create();
        }
        catch ( Exception e ) {
            logFatal ( e.toString() );
            throw new EJBException ( "Error in ProjectionFacade:setMessageDrivenContext " + e.toString() );
        }
    }

    /**
     *  Standard EJB implementation
     */
    public void ejbCreate() { }

    /**
     *  Standard EJB implementation
     */
    public void ejbRemove() {
        mCTX = null;
    }

    /**
     * onMessage is executed on callback from the EJ Bean container whenever
     * a JMS message is sent to the queue on which ProjectionFacade is bound.
     * Messages send to ProjectionFacade are used to invoke group projections
     * in an aysnchronous manner.
     *
     * The message has to include the following bits of information as mapMessage
     * fields:
     *          "entitytype" - the type of entity for which this projection is
     * intended.
     *          "entityid" - the PK of the particular entity you want to
     * project for.
     *          "profiletypes" - a comma-separated list of profiles which need
     * to be projected.
     *
     * @param message JMS Message taken from the queue
     */
    public void onMessage( Message message ) {

        MapMessage mapMessage = (MapMessage) message;

        try {
            String entityType = mapMessage.getString( "entitytype" );
            String entityID = mapMessage.getString( "entityid" );
            String profileTypes = mapMessage.getString( "profiletypes" );

            if ( entityType.equals( "usergroup") ) {
                logDebug ( "projection facade: generating profiles for user group: " + entityID );
                mUserGroupAdvocate.generateProfiles( entityID, profileTypes, null );
            }
            else if ( entityType.equals( "devicegroup") ) {
                logDebug ( "projection facade: generating profiles for device group: " + entityID );
                mDeviceGroupAdvocate.generateProfiles( entityID, profileTypes, null );
            }
        }
        catch (RemoteException e) {
            logFatal ( e.toString() );
        }
        catch (JMSException e) {
            logFatal ( e.toString() );
        }
        catch (PDSException e) {
            logFatal ( e.toString() );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////


    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}